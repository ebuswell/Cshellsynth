/*
 * pan.c
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
#include "cshellsynth/pan.h"
#include "cshellsynth/jclient.h"
#include "atomic-float.h"

static int cs_pan_process(jack_nframes_t nframes, void *arg) {
    cs_pan_t *self = (cs_pan_t *) arg;
    float *in_buffer = (float *) jack_port_get_buffer(self->in_port, nframes);
    if(in_buffer == NULL) {
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
    float pan = atomic_float_read(&self->pan);
    float *pan_buffer = pan_buffer;
    if(isnanf(pan)) {
	pan_buffer = (float *) jack_port_get_buffer(self->pan_port, nframes);
	if(pan_buffer == NULL) {
	    return -1;
	}
    }
    jack_nframes_t i;
    for(i = 0; i < nframes; i++) {
	float c_pan = isnanf(pan) ? pan_buffer[i] : pan;
	if(c_pan < 0.0f) {
	    outL_buffer[i] = in_buffer[i];
	    outR_buffer[i] = (1.0f + c_pan) * in_buffer[i];
	} else {
	    outR_buffer[i] = in_buffer[i];
	    outL_buffer[i] = (1.0f - c_pan) * in_buffer[i];
	}
    }
    return 0;
}

void cs_pan_set_pan(cs_pan_t *self, float pan) {
    atomic_float_set(&self->pan, pan);
}

int cs_pan_init(cs_pan_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = jclient_init((jclient_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->in_port = jack_port_register(self->client, "in", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->in_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return -1;
    }

    self->pan_port = jack_port_register(self->client, "pan", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->pan_port == NULL) {
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

    atomic_float_set(&self->pan, NAN);

    r = jack_set_process_callback(self->client, cs_pan_process, self);
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
