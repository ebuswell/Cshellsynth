#include <jack/jack.h>
#include <math.h>
#include <sndfile.h>
#include <alloca.h>
#include <string.h>
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
    SNDFILE *sf = atomic_ptr_read(&self->sf);
    float *interleaved = NULL;
    if(sf != NULL) {
	jack_nframes_t i = 0;
	while(i < nframes) {
	    if(ctl_buffer[i] == -1.0f) {
		self->playing = false;
	    } else if(ctl_buffer[i] == 1.0f) {
		sf_count_t r = sf_seek(sf, 0, SEEK_SET);
		self->playing = true;
		if(r < 0) {
		    return r;
		}
	    }
	    if(self->playing) {
		if(interleaved == NULL) {
		    interleaved = alloca(nframes * sizeof(float) * 2);
		    memset(interleaved, 0, nframes * sizeof(float) * 2);
		}
		/* find out how long we're playing... */
		int j;
		for(j = i + 1; j < nframes; j++) {
		    if(ctl_buffer[j] != 0.0f) {
			break;
		    }
		}
		sf_count_t c = sf_readf_float(sf, (float *) interleaved + i * 2, j - i);
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
	    for(i = 0; i < nframes; i ++) {
		outL_buffer[i] = interleaved[2*i];
		outR_buffer[i] = interleaved[2*i + 1];
	    }
	} else {
	    memset(outL_buffer, 0, sizeof(float) * nframes);
	    memset(outR_buffer, 0, sizeof(float) * nframes);
	}
    } else {
	memset(outL_buffer, 0, sizeof(float) * nframes);
	memset(outR_buffer, 0, sizeof(float) * nframes);
    }
    return 0;
}

int cs_sampler_load(cs_sampler_t *self, char *path) {
    SNDFILE *sf;
    SF_INFO sf_info;
    int r;
    memset(&sf_info, 0, sizeof(sf_info));
    sf = sf_open(path, SFM_READ, &sf_info);
    if(sf == NULL) {
	return -1;
    }
    if(sf_info.samplerate != jack_get_sample_rate(self->client)) {
	r = sf_close(sf);
	if(r != 0) {
	    return r;
	}
	return -1;
    }
    if(sf_info.channels != 2) {
	r = sf_close(sf);
	if(r != 0) {
	    return r;
	}
	return -1;
    }

    sf = atomic_ptr_xchg(&self->sf, sf);
    if(sf != NULL) {
	r = sf_close(sf);
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
