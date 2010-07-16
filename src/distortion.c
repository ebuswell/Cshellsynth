/*
 * distortion.c
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
#include "cshellsynth/distortion.h"
#include "cshellsynth/filter.h"
#include "atomic-float.h"

static int cs_distort_process(jack_nframes_t nframes, void *arg) {
    cs_distort_t *self = (cs_distort_t *) arg;
    float *in_buffer;
    float *gain_buffer;
    float *out_buffer = (float *)jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    float in = atomic_float_read(&self->in);
    if(isnanf(in)) {
	in_buffer = (float *)jack_port_get_buffer(self->in_port, nframes);
	if(in_buffer == NULL) {
	    return -1;
	}
    }
    float gain = atomic_float_read(&self->gain);
    if(isnanf(gain)) {
	gain_buffer = (float *)jack_port_get_buffer(self->gain_port, nframes);
	if(gain_buffer == NULL) {
	    return -1;
	}
    }
    double sharpness = (double) atomic_float_read(&self->sharpness);
    double factor = log(exp(sharpness) + 1.0);
    int i;
    for(i = 0; i < nframes; i++) {
	double c_in = (double) (isnanf(in) ? in_buffer[i] : in);
	double c_gain = (double) (isnanf(gain) ? gain_buffer[i] : gain);
	if(c_in >= 0.0f) {
	    out_buffer[i] = (float) (
		1.0
		- log(exp(-sharpness * ((c_in * c_gain) - 1.0)) + 1.0)
		  / factor);
	} else {
	    out_buffer[i] = (float) (
		log(exp(sharpness * ((c_in * c_gain) + 1.0)) + 1.0)
		/ factor
		- 1.0);
	}
    }
    return 0;
}

void cs_distort_set_gain(cs_distort_t *self, float gain) {
    atomic_float_set(&self->gain, gain);
}

void cs_distort_set_sharpness(cs_distort_t *self, float sharpness) {
    atomic_float_set(&self->sharpness, sharpness);
}

int cs_distort_init(cs_distort_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_filter_init((cs_filter_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->gain_port = jack_port_register(self->client, "gain", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->gain_port == NULL) {
	cs_filter_destroy((cs_filter_t *) self);
	return -1;
    };

    atomic_float_set(&self->gain, 1.0f);
    atomic_float_set(&self->sharpness, 2.0f);

    r = jack_set_process_callback(self->client, cs_distort_process, self);
    if(r != 0) {
	cs_filter_destroy((cs_filter_t *) self);
	return r;
    }

    r = jack_activate(self->client);
    if(r != 0) {
	cs_filter_destroy((cs_filter_t *) self);
	return r;
    }

    return 0;
}
