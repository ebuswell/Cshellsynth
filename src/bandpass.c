/*
 * bandpass.c
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
#include "cshellsynth/bandpass.h"
#include "cshellsynth/filter.h"
#include "atomic-float.h"
#include "atomic-double.h"

static int cs_bandpass_process(jack_nframes_t nframes, void *arg) {
    cs_bandpass_t *self = (cs_bandpass_t *) arg;
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
    jack_nframes_t i;
    for(i = 0; i < nframes; i++) {
	double f = isnanf(freq) ? freq_buffer[i] : freq;
	if(f > 1.0) {
	    f = 1.0;
	}
	double w = 2.0 * M_PI * f;
	double cQ = isnanf(Q) ? Q_buffer[i] : Q;
	double out = (((double) (isnanf(in) ? in_buffer[i] : in)) * w
		      + self->last_out * cQ * (1 + w * w)
		      - self->out_accumulator * cQ * w * w)
	    / (w + cQ + cQ * w * w);
	self->out_accumulator += self->last_out;
	if(self->out_accumulator == INFINITY) {
	    self->out_accumulator = HUGE;
	} else if(self->out_accumulator == -INFINITY) {
	    self->out_accumulator = -HUGE;
	}
	self->last_out = out;
	out_buffer[i] = out;
    }
    return 0;
}

void cs_bandpass_set_freq(cs_bandpass_t *self, float freq) {
    if(freq > 1.0) {
	freq = freq / jack_get_sample_rate(self->client);
    }
    atomic_float_set(&self->freq, freq);
}

void cs_bandpass_set_Q(cs_bandpass_t *self, float Q) {
    atomic_float_set(&self->Q, Q);
}

int cs_bandpass_init(cs_bandpass_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_filter_init((cs_filter_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->freq_port = jack_port_register(self->client, "freq", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->freq_port == NULL) {
	cs_filter_destroy((cs_filter_t *) self);
	return -1;
    };

    self->Q_port = jack_port_register(self->client, "Q", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->Q_port == NULL) {
	cs_filter_destroy((cs_filter_t *) self);
	return -1;
    };

    atomic_float_set(&self->freq, NAN);
    atomic_float_set(&self->Q, 0.5);
    self->last_out = 0.0;
    self->out_accumulator = 0.0;

    r = jack_set_process_callback(self->client, cs_bandpass_process, self);
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
