/*
 * dsf.c Discrete Summation Formula
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
#include "cshellsynth/dsf.h"
#include "cshellsynth/synth.h"
#include "cshellsynth/jclient.h"
#include "atomic-float.h"
#include "atomic.h"
#include "util.h"

static int cs_dsf_process(jack_nframes_t nframes, void *arg) {
    cs_dsf_t *self = (cs_dsf_t *) arg;
    float *freq_buffer = freq_buffer; /* suppress uninitialized warning */
    float *bright_buffer = bright_buffer; /* suppress uninitialized warning */
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
    float bright = atomic_float_read(&self->bright);
    if(isnanf(bright)) {
	bright_buffer = (float *) jack_port_get_buffer(self->bright_port, nframes);
	if(bright_buffer == NULL) {
	    return -1;
	}
    }
    float offset = atomic_float_read(&self->offset);
    float amp = atomic_float_read(&self->amp);
    int scale = atomic_read(&self->scale);
    jack_nframes_t i;
    for(i = 0; i < nframes; i++) {
	double f = (double) (isnanf(freq) ? freq_buffer[i] : freq);
	double m = (double) (isnanf(bright) ? bright_buffer[i] : bright);
	if(f == 0.0 || isnan(f)) {
	    self->t = 0.0;
	    out_buffer[i] = offset;
	} else {
	    if(self->t >= 1.0) {
		self->t -= 1.0;
	    }
	    double out;
	    double n = floor(1.0 / (2.0 * f));
	    double na = (0.5 - n*f) / 0.0003;
	    double wt = 2.0 * M_PI * self->t;
	    if(scale) {
		/* the reason it's different when we scale is because eventually the peak
		 * is not visible within the sample resolution.  This value should keep
		 * everything within reasonable bounds from 15 hz on (at 44100kHz).  A
		 * higher value could be used for higher frequencies, but that makes this
		 * value frequency dependent, which doesn't sound good.  I suppose this
		 * could probably be coded according to sample-rate, but then that would
		 * make the brightness of 1.0 dependent on sample frequency in an
		 * unexpected way.  Formula is (cos(4pi(15/sample_rate))/(1 +
		 * sin(4pi(15/sample_rate))) - 3.0517578125e-05 */
		m = m * 0.9957043154589819 + 3.0517578125e-05;
		if(n == 1.0) {
		    out = sin(wt) * (1.0 - m * m);
		} else {
		    out = (sin(wt) - pow(m,n) * (sin((n + 1.0) * wt) - m * sin(n * wt)))
			/ (1.0 + m*m - 2.0*m*cos(wt));
		    if(na < 1.0) {
			out -= (1.0 - L2ESCALE(na))*pow(m,n)*sin(n * wt);
		    }
		    out *= (1.0 - m * m);
		}
	    } else {
		if(n == 1.0) {
		    out = sin(wt);
		} else {
		    out = (sin(wt) - pow(m,n) * (sin((n + 1.0) * wt) - m * sin(n * wt)))
			/ (1.0 + m*m - 2.0*m*cos(wt));
		    if(na < 1.0) {
			out -= (1.0 - L2ESCALE(na)) * pow(m,n) * sin(n * wt);
		    }
		}
	    }
	    out_buffer[i] = out * amp + offset;
	    self->t += f;
	}
    }
    return 0;
}

void cs_dsf_set_bright(cs_dsf_t *self, float bright) {
    atomic_float_set(&self->bright, bright);
}

void cs_dsf_set_scale(cs_dsf_t *self, int scale) {
    atomic_set(&self->scale, scale);
}

int cs_dsf_init(cs_dsf_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_synth_init((cs_synth_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }
    self->bright_port = jack_port_register(self->client, "bright", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->bright_port == NULL) {
	cs_synth_destroy((cs_synth_t *) self);
	return -1;
    }

    atomic_set(&self->scale, 0);
    atomic_float_set(&self->bright, 0.5);
    self->t = 0.0;
    r = jack_set_process_callback(self->client, cs_dsf_process, self);
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
