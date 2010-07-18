/*
 * square.c
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
#include "cshellsynth/square.h"
#include "cshellsynth/synth.h"
#include "cshellsynth/jclient.h"
#include "atomic-float.h"
#include "atomic-double.h"

static int cs_square_process(jack_nframes_t nframes, void *arg) {
    cs_square_t *self = (cs_square_t *) arg;
    float *freq_buffer = freq_buffer; /* suppress uninitialized warning */
    float *out_buffer = (float *) jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    double duty_cycle = atomic_double_read(&self->duty_cycle);
    float freq = atomic_float_read(&self->freq);
    if(isnanf(freq)) {
	freq_buffer = (float *) jack_port_get_buffer(self->freq_port, nframes);
	if(freq_buffer == NULL) {
	    return -1;
	}
    }
    float offset = atomic_float_read(&self->offset);
    float amp = atomic_float_read(&self->amp);
    jack_nframes_t i;
    for(i = 0; i < nframes; i++) {
	float f = isnanf(freq) ? freq_buffer[i] : freq;
	if(f == 0.0f || isnanf(f)) {
	    self->t = 0.0;
	    out_buffer[i] = offset;
	} else {
	    while(self->t >= 1.0) {
		self->t -= 1.0;
	    }
	    if(self->t >= duty_cycle) {
		if(self->t + f > 1.0) {
		    // percentage of sample before transition
		    double diff = (1.0 - self->t) / f;
		    // average value is (1.0 * diff) + (-1.0 * (1 - diff))
		    // diff - 1 + diff
		    // 2*diff - 1
		    out_buffer[i] = ((2.0 * diff) - 1.0f) * amp + offset;
		} else {
		    out_buffer[i] = 1.0f * amp + offset;
		}
	    } else {
		if(self->t + f > duty_cycle) {
		    // percentage of sample before transition
		    float diff = (duty_cycle - self->t) / f;
		    // average value is (-1.0 * diff) + (1.0 * (1 - diff))
		    // -diff + 1 - diff
		    // 1 - 2*diff
		    out_buffer[i] = (1.0f - (2.0 * diff)) * amp + offset;
		} else {
		    out_buffer[i] = (-1.0f) * amp + offset;
		}
	    }
	    self->t += f;
	}
    }
    return 0;
}

void cs_square_set_duty_cycle(cs_square_t *self, double duty_cycle) {
    atomic_double_set(&self->duty_cycle, duty_cycle);
}

int cs_square_init(cs_square_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_synth_init((cs_synth_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }
    r = jack_set_process_callback(self->client, cs_square_process, self);
    if(r != 0) {
	cs_synth_destroy((cs_synth_t *) self);
	return r;
    }
    self->t = 0.0;
    atomic_double_set(&self->duty_cycle, 0.5);
    r = jack_activate(self->client);
    if(r != 0) {
	cs_synth_destroy((cs_synth_t *) self);
	return r;
    }
    return 0;
}
