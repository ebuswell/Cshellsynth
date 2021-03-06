/*
 * falling_saw.c
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
#include "cshellsynth/falling_saw.h"
#include "falling_saw.h"
#include "cshellsynth/synth.h"
#include "cshellsynth/jclient.h"
#include "atomic-float.h"

#define CS_FSAW_MAGIC 7.52134027348215

inline double cs_fsaw_exec(double t, double n, double na) {
    double wt = 2.0 * M_PI * t;
    double out;
    if(na < 1.0) {
	out = -(1.0 - F2ESCALE(na)) * sin(n * wt) / n;
    } else {
	out = 0.0;
    }
    if(n <= 5) {
	double j;
	for(j = 1.0; j <= n; j += 1.0) {
	    out += sin(j * wt) / j;
	}
	return out;
    } else {
	double sign = signbit(0.5 - t);
	t = (1 - fabs(1 - 2 * t)) / 2; /* keeps t between 0 and 0.5 */
	double n_21 = 2.0 * n + 1.0;
	double cf = (M_PI / (1.0 - 2.0 / (n_21 * CS_FSAW_MAGIC)));
	double s_off_d = M_PI / asin(2.0 * cos(cf / CS_FSAW_MAGIC) / (M_PI * n_21)) - 2.0;
	double adj = cos(cf * (n_21 * t - 1.0 / CS_FSAW_MAGIC)) / (n_21 * sin(M_PI * (t + 1.0 / s_off_d) / (1.0 + 2.0 / s_off_d)));
	double raw = M_PI * (0.5 - t);
	return out + sign * (raw - adj);
    }
}

static int cs_fsaw_process(jack_nframes_t nframes, void *arg) {
    cs_fsaw_t *self = (cs_fsaw_t *) arg;
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
    jack_nframes_t i;
    for(i = 0; i < nframes; i++) {
	float f = isnanf(freq) ? freq_buffer[i] : freq;
	if(f == 0.0f || isnanf(f)) {
	    self->t = 0.0;
	    out_buffer[i] = 0.0;
	} else {
	    while(self->t >= 1.0) {
		self->t -= 1.0;
	    }
	    double n = floor(1.0 / (2.0 * f));
	    double na = (0.5 - n*f) / 0.0003;
	    out_buffer[i] = cs_fsaw_exec(self->t, n, na) * amp + offset;
	    self->t += f;
	}
    }
    return 0;
}

int cs_fsaw_init(cs_fsaw_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_synth_init((cs_synth_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }
    r = jack_set_process_callback(self->client, cs_fsaw_process, self);
    if(r != 0) {
	cs_synth_destroy((cs_synth_t *) self);
	return r;
    }
    self->t = 0.0;
    r = jack_activate(self->client);
    if(r != 0) {
	cs_synth_destroy((cs_synth_t *) self);
	return r;
    }
    return 0;
}
