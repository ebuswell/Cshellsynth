/*
 * clock.c
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
#include "cshellsynth/clock.h"
#include "cshellsynth/jclient.h"
#include "atomic-float.h"

void cs_clock_set_meter(cs_clock_t *self, float meter) {
    atomic_float_set(&self->meter, meter);
}

void cs_clock_set_rate(cs_clock_t *self, float rate) {
    if(rate > 1.0f) {
        /* convert bpm to b/sample */
	atomic_float_set(&self->rate, rate / (60.0 * ((double) jack_get_sample_rate(self->client))));
    } else {
	atomic_float_set(&self->rate, rate);
    }
}

static int cs_clock_process(jack_nframes_t nframes, void *arg) {
    cs_clock_t *self = (cs_clock_t *) arg;
    float *rate_buffer = rate_buffer; /* suppress uninitialized warning */
    float *meter_buffer = meter_buffer; /* suppress uninitialized warning */
    float *clock_buffer = (float *)jack_port_get_buffer(self->clock_port, nframes);
    if(clock_buffer == NULL) {
	return -1;
    }
    float meter = atomic_float_read(&self->meter);
    if(isnanf(meter)) {
	meter_buffer = (float *)jack_port_get_buffer(self->meter_port, nframes);
	if(meter_buffer == NULL) {
	    return -1;
	}
    }
    float rate = atomic_float_read(&self->rate);
    if(isnanf(rate)) {
	rate_buffer = (float *)jack_port_get_buffer(self->rate_port, nframes);
	if(rate_buffer == NULL) {
	    return -1;
	}
    }
    jack_nframes_t i;
    for(i = 0; i < nframes; i++) {
	double c_meter = isnanf(meter) ? meter_buffer[i] : meter;
	while(self->current >= c_meter) {
	    self->current -= c_meter;
	}
	clock_buffer[i] = (float) self->current;
	self->current += (double) (isnanf(rate) ? rate_buffer[i] : rate);
    }
    return 0;
}

int cs_clock_init(cs_clock_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = jclient_init((jclient_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->clock_port = jack_port_register(self->client, "clock", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(self->clock_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return -1;
    }

    self->meter_port = jack_port_register(self->client, "meter", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->meter_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return -1;
    }

    self->rate_port = jack_port_register(self->client, "rate", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->rate_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return -1;
    }

    self->current = 0.0;
    atomic_float_set(&self->meter, 4.0f);
    atomic_float_set(&self->rate, 120.0 / (60.0 * ((double) jack_get_sample_rate(self->client))));

    r = jack_set_process_callback(self->client, cs_clock_process, self);
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
