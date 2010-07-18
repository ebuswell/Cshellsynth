/*
 * lin2exp.c
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
#include "cshellsynth/lin2exp.h"
#include "cshellsynth/filter.h"
#include "atomic-float.h"

static int cs_lin2exp_process(jack_nframes_t nframes, void *arg) {
    cs_lin2exp_t *self = (cs_lin2exp_t *) arg;
    float *in_buffer = in_buffer; /* suppress uninitialized warning */
    float *zero_buffer = zero_buffer; /* suppress uninitialized warning */
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
    float zero = atomic_float_read(&self->zero);
    if(isnanf(zero)) {
	zero_buffer = (float *)jack_port_get_buffer(self->zero_port, nframes);
	if(zero_buffer == NULL) {
	    return -1;
	}
    }
    jack_nframes_t i;
    for(i = 0; i < nframes; i++) {
	out_buffer[i] = ((double) (isnanf(zero) ? zero_buffer[i] : zero))*pow(2.0, (double) (isnanf(in) ? in_buffer[i] : in));
    }
    return 0;
}

void cs_lin2exp_set_zero(cs_lin2exp_t *self, float zero) {
    atomic_float_set(&self->zero, zero);
}

int cs_lin2exp_init(cs_lin2exp_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_filter_init((cs_filter_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }
    self->zero_port = jack_port_register(self->client, "zero", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->zero_port == NULL) {
	cs_filter_destroy((cs_filter_t *) self);
	return -1;
    }

    atomic_float_set(&self->zero, NAN);

    r = jack_set_process_callback(self->client, cs_lin2exp_process, self);
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
