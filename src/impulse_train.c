/*
 * impulse_train.c Discrete Summation Formula
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
#include "cshellsynth/impulse_train.h"
#include "cshellsynth/synth.h"
#include "cshellsynth/jclient.h"
#include "atomic-float.h"
#include "atomic.h"
#include "util.h"

static int cs_itrain_process(jack_nframes_t nframes, void *arg) {
    cs_itrain_t *self = (cs_itrain_t *) arg;
    float *freq_buffer = freq_buffer; /* suppress uninitialized warning */
    float *out_buffer = (float *) jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    float freq = atomic_float_read(&self->freq);
    if(isnanf(freq)) {
	freq_buffer = (float *) jack_port_get_buffer(self->freq_port, nframes);
	if(freq_buffer == NULL) {
	    return -1;
	}
    }
    float offset = atomic_float_read(&self->offset);
    float amp = atomic_float_read(&self->amp);
    int scale = atomic_read(&self->scale);
    jack_nframes_t i;
    for(i = 0; i < nframes; i++) {
	double f = (double) (isnanf(freq) ? freq_buffer[i] : freq);
	if(f == 0.0 || isnan(f)) {
	    self->t = 0.0;
	    out_buffer[i] = offset;
	} else {
	    if(self->t >= 1.0) {
		self->t -= 1.0;
	    }
	    double n = floor(1.0 / (2.0 * f));
	    double na = (0.5 - n*f) / 0.0003;
	    double wt = M_PI * self->t;
	    double out = sin(n * wt) * cos((n + 1.0) * wt)
		/ sin(wt);
	    if(na < 1.0) {
		out -= (1.0 - L2ESCALE(na))*cos(n * 2.0 * wt);
	    }
	    if(scale) {
		out *= 2.0 * f;
	    }
	    out_buffer[i] = out * amp + offset;
	    self->t += f;
	}
    }
    return 0;
}

void cs_itrain_set_scale(cs_itrain_t *self, int scale) {
    atomic_set(&self->scale, scale);
}

int cs_itrain_init(cs_itrain_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_synth_init((cs_synth_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    atomic_set(&self->scale, 0);
    self->t = 0.0;
    r = jack_set_process_callback(self->client, cs_itrain_process, self);
    if(r != 0) {
	cs_synth_destroy((cs_synth_t *) self);
	return r;
    }
    r = jack_activate(self->client);
    if(r != 0) {
	cs_synth_destroy((cs_synth_t *) self);
	return r;
    }
    return 0;
}
