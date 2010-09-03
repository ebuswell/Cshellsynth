/*
 * highpass.c
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
#include "cshellsynth/highpass.h"
#include "cshellsynth/filter.h"
#include "atomic-float.h"

static int cs_highpass_process(jack_nframes_t nframes, void *arg) {
    cs_highpass_t *self = (cs_highpass_t *) arg;
    float *in_buffer = in_buffer; /* suppress uninitialized warning */
    float *freq_buffer = freq_buffer; /* suppress uninitialized warning */
    float *Q_buffer = Q_buffer; /* suppress uninitialized warning */
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
    float freq = atomic_float_read(&self->freq);
    if(isnanf(freq)) {
	freq_buffer = (float *)jack_port_get_buffer(self->freq_port, nframes);
	if(freq_buffer == NULL) {
	    return -1;
	}
    }
    float Q = atomic_float_read(&self->Q);
    if(isnanf(Q)) {
	Q_buffer = (float *)jack_port_get_buffer(self->Q_port, nframes);
	if(Q_buffer == NULL) {
	    return -1;
	}
    }
    float a = atomic_float_read(&self->atten);
    jack_nframes_t i;
    for(i = 0; i < nframes; i++) {
	double f = isnanf(freq) ? freq_buffer[i] : freq;
	if(f > 0.5) {
	    f = 0.5;
	}
	double x = isnanf(in) ? in_buffer[i] : in;
	double w = 2.0 * M_PI * f;
	if(isnanf(a)) {
	    a = sin(w)/(2*(isnanf(Q) ? Q_buffer[i] : Q));
	}
	double cosw = cos(w);
	double y = (((1.0 + cosw)/2)/(1.0 + a)) * x - ((1.0 + cosw)/(1.0 + a)) * self->x1 + (((1.0 + cosw)/2)/(1.0 + a)) * self->x2
	    + (2*cosw/(1.0 + a)) * self->y1 - ((1 - a)/(1 + a)) * self->y2;

	self->x2 = self->x1;
	self->x1 = x;
	self->y2 = self->y1;
	if(y == INFINITY) {
	    self->y1 = 1.0;
	} else if (y == -INFINITY) {
	    self->y1 = -1.0;
	} else {
	    self->y1 = y;
	}
	out_buffer[i] = y;
    }
    return 0;
}

int cs_highpass_init(cs_highpass_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_lowpass_subclass_init((cs_lowpass_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }
    r = jack_set_process_callback(self->client, cs_highpass_process, self);
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
