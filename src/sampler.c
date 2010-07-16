/*
 * sampler.c
 * 
 * Copyright 2010 Evan Buswell
 * 
 * This file is part of Cshellsynth.
 * 
 * Cshellsynth is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Cshellsynth is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Cshellsynth.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <jack/jack.h>
#include <math.h>
#include <sndfile.h>
#include <alloca.h>
#include <string.h>
#include <stdlib.h>
#include "cshellsynth/sampler.h"
#include "cshellsynth/jclient.h"
#include "atomic-ptr.h"

static int cs_sampler_process(jack_nframes_t nframes, void *arg) {
    cs_sampler_t *self = (cs_sampler_t *) arg;
    float *ctl_buffer = (float *) jack_port_get_buffer(self->ctl_port, nframes);
    if(ctl_buffer == NULL) {
	return -1;
    }
    float *outL_buffer = (float *) jack_port_get_buffer(self->outL_port, nframes);
    if(outL_buffer == NULL) {
	return -1;
    }
    float *outR_buffer = (float *) jack_port_get_buffer(self->outR_port, nframes);
    if(outR_buffer == NULL) {
	return -1;
    }
    atomic_inc(&self->sf_sync);
    cs_sampler_sf_t *sf = atomic_ptr_read(&self->sf);
    float *interleaved = NULL;
    if(sf != NULL) {
	jack_nframes_t i = 0;
	while(i < nframes) {
	    if(ctl_buffer[i] == 1.0f) {
		sf_count_t r = sf_seek(sf->sf, 0, SEEK_SET);
		if(r < 0) {
		    return r;
		}
		self->playing = true;
	    }
	    if(self->playing) {
		if(interleaved == NULL) {
		    interleaved = alloca(nframes * sizeof(float) * sf->sf_info.channels);
		    memset(interleaved, 0, nframes * sizeof(float) * sf->sf_info.channels);
		}
		/* find out how long we're playing... */
		int j;
		for(j = i + 1; j < nframes; j++) {
		    if(ctl_buffer[j] == 1.0f) {
			break;
		    }
		}
		sf_count_t c = sf_readf_float(sf->sf, (float *) interleaved + i * sf->sf_info.channels, j - i);
		if(c < 0) {
		    return c;
		}
		if(c < j - i) {
		    /* EOF */
		    self->playing = false;
		}
		i = j;
	    } else {
		/* find out when we start playing... */
		for(; i < nframes; i++) {
		    if(ctl_buffer[i] == 1.0f) {
			break;
		    }
		}
	    }
	}
	if(interleaved != NULL) {
	    if(sf->sf_info.channels == 1) {
		for(i = 0; i < nframes; i ++) {
		    outL_buffer[i] = outR_buffer[i] = interleaved[i];
		}
	    } else {
		for(i = 0; i < nframes; i ++) {
		    outL_buffer[i] = interleaved[sf->sf_info.channels*i];
		    outR_buffer[i] = interleaved[sf->sf_info.channels*i + 1];
		}
	    }
	} else {
	    memset(outL_buffer, 0, sizeof(float) * nframes);
	    memset(outR_buffer, 0, sizeof(float) * nframes);
	}
    } else {
	memset(outL_buffer, 0, sizeof(float) * nframes);
	memset(outR_buffer, 0, sizeof(float) * nframes);
    }
    atomic_dec(&self->sf_sync);
    return 0;
}

int cs_sampler_load(cs_sampler_t *self, char *path) {
    cs_sampler_sf_t *sf = malloc(sizeof(cs_sampler_sf_t));
    if(sf == NULL) {
	return -1;
    }
    int r;
    memset(&sf->sf_info, 0, sizeof(SF_INFO));
    sf->sf = sf_open(path, SFM_READ, &sf->sf_info);
    if(sf->sf == NULL) {
	free(sf);
	return -1;
    }
    if(sf->sf_info.samplerate != jack_get_sample_rate(self->client)) {
	r = sf_close(sf->sf);
	free(sf);
	if(r != 0) {
	    return r;
	}
	return -1;
    }

    sf = atomic_ptr_xchg(&self->sf, sf);
    if(sf != NULL) {
	/* make sure we're not using it before we free it */
	while(atomic_read(&self->sf_sync)) {
	    sched_yield();
	}
	r = sf_close(sf->sf);
	free(sf);
	if(r != 0) {
	    return r;
	}
    }
    return 0;
}

int cs_sampler_destroy(cs_sampler_t *self) {
    int r = jclient_destroy((jclient_t *) self);
    if(atomic_ptr_read(&self->sf) != NULL) {
	r |= sf_close(atomic_ptr_read(&self->sf));
    }
    if(r != 0) {
	return r;
    }
    return 0;
}

int cs_sampler_init(cs_sampler_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = jclient_init((jclient_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->ctl_port = jack_port_register(self->client, "ctl", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->ctl_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return -1;
    }

    self->outL_port = jack_port_register(self->client, "outL", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(self->outL_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return -1;
    }

    self->outR_port = jack_port_register(self->client, "outR", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(self->outR_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return -1;
    }

    atomic_ptr_set(&self->sf, NULL);
    atomic_set(&self->sf_sync, 0);
    self->playing = false;

    r = jack_set_process_callback(self->client, cs_sampler_process, self);
    if(r != 0) {
	jclient_destroy((jclient_t *) self);
	return r;
    }

    r = jack_activate(self->client);
    if(r != 0) {
	jclient_destroy((jclient_t *) self);
	return r;
    }

    return 0;
}
