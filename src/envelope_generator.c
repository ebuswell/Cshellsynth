/*
 * envelope_generator.c
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
#include <stdbool.h>
#include "cshellsynth/envelope_generator.h"
#include "cshellsynth/jclient.h"
#include "atomic-float.h"
#include "atomic-double.h"

static inline double cs_envg_process_stage(int linear, double from, double to, double last, double time) {
    if(linear) {
	return last + (to - from) / time;
    } else {
	return to + (last - to) * exp(-M_PI / time);
    }
}

static int cs_envg_process(jack_nframes_t nframes, void *arg) {
    cs_envg_t *self = (cs_envg_t *) arg;
    float *ctl_buffer = (float *) jack_port_get_buffer(self->ctl_port, nframes);
    if(ctl_buffer == NULL) {
	return -1;
    }
    float *out_buffer = (float *) jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    double attack_t = atomic_double_read(&self->attack_t);
    float attack_a = atomic_float_read(&self->attack_a);
    double decay_t = atomic_double_read(&self->decay_t);
    float sustain_a = atomic_float_read(&self->sustain_a);
    double release_t = atomic_double_read(&self->release_t);
    float release_a = atomic_float_read(&self->release_a);
    int linear = atomic_read(&self->linear);
    double attack_adj;
    if(!linear) {
	/* Adjust attack_a */
	attack_adj = (((double) attack_a) - ((double) release_a) * exp(-M_PI))
		   /*---------------------------------------------------------*/
		   /                  (1.0 - exp(-M_PI));
    }
    int i;
    for(i = 0; i < nframes; i++) {
	float ctl = ctl_buffer[i];
	if(isnanf(ctl)) {
	    ctl = 0.0f;
	}
	if(ctl > 0.0f) {
	    // attack event
	    self->state = ATTACK;
	    self->release = false;
	    self->upwards = (attack_a > self->last_a);
	} else if(ctl < 0.0f) {
	    // release event
	    self->release = true;
	}
	switch(self->state) {
	case ATTACK:
	    if(attack_t <= 0.0) {
		self->last_a = attack_a;
		self->state = DECAY;
		self->upwards = (sustain_a > self->last_a);
		// fall through
	    } else {
		out_buffer[i] = self->last_a = cs_envg_process_stage(linear, release_a, linear ? attack_a : attack_adj, self->last_a, attack_t);
		if(self->upwards ? (self->last_a >= attack_a) : (self->last_a <= attack_a)) {
		    out_buffer[i] = self->last_a = attack_a;
		    self->state = DECAY;
		    self->upwards = (sustain_a > self->last_a);
		}
		break;
	    }
	case DECAY:
	    if(decay_t <= 0.0) {
		self->state = SUSTAIN;
		// fall through
	    } else {
		out_buffer[i] = self->last_a = cs_envg_process_stage(linear, attack_a, sustain_a, self->last_a, decay_t);
		if(linear) {
		    if(self->upwards ? (self->last_a >= sustain_a) : (self->last_a <= sustain_a)) {
			self->state = SUSTAIN;
			// fall through
		    } else {
			break;
		    }
		} else {
		    // find out if we are within the range for releasing:
		    if(self->release
		       && (self->upwards
			   ? (self->last_a >= ((double) sustain_a) 
			      + ((double) (attack_a - sustain_a)) * exp(-M_PI))
			   : (self->last_a <= ((double) sustain_a)
			      + ((double) (attack_a - sustain_a)) * exp(-M_PI)))) {
			self->state = RELEASE;
			self->upwards = (release_a > self->last_a);
			// fall through
		    } else {
			if(self->upwards ? (self->last_a >= sustain_a) : (self->last_a <= sustain_a)) {
			    self->state = SUSTAIN;
			    // fall through
			} else {
			    break;
			}
		    }
		}
	    }
	case SUSTAIN:
	    // check fall through
	    if(self->state == SUSTAIN) {
		out_buffer[i] = self->last_a = sustain_a;
		if(self->release) {
		    self->state = RELEASE;
		    self->upwards = (release_a > self->last_a);
		    // fall through
		} else {
		    break;
		}
	    }
	case RELEASE:
	    if(release_t <= 0.0) {
		self->state = FINISHED;
		// fall through
	    } else {
		out_buffer[i] = self->last_a = cs_envg_process_stage(linear, sustain_a, release_a, self->last_a, release_t);
		if(self->upwards ? (self->last_a >= release_a) : (self->last_a <= release_a)) {
		    self->state = FINISHED;
		    // fall through
		} else {
		    break;
		}
	    }
	case FINISHED:
	    out_buffer[i] = self->last_a = release_a;
	}
    }
    return 0;
}

int cs_envg_init(cs_envg_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = jclient_init((jclient_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->ctl_port = jack_port_register(self->client, "ctl", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->ctl_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return -1;
    }

    self->out_port = jack_port_register(self->client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(self->out_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return -1;
    }

    atomic_double_set(&self->attack_t, 0.0);
    atomic_float_set(&self->attack_a, 1.0f);
    atomic_double_set(&self->decay_t, 0.0);
    atomic_float_set(&self->sustain_a, 1.0f);
    atomic_double_set(&self->release_t, 0.0);
    atomic_float_set(&self->release_a, 0.0);
    atomic_set(&self->linear, 0);
    self->state = FINISHED;
    self->last_a = 0.0;
    self->release = false;

    r = jack_set_process_callback(self->client, cs_envg_process, self);
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

void cs_envg_set_linear(cs_envg_t *self, int linear) {
    atomic_set(&self->linear, linear);
}

void cs_envg_set_attack_t(cs_envg_t *self, double attack_t) {
    atomic_double_set(&self->attack_t, attack_t * jack_get_sample_rate(self->client));
}

void cs_envg_set_attack_a(cs_envg_t *self, float attack_a) {
    atomic_float_set(&self->attack_a, attack_a);
}

void cs_envg_set_decay_t(cs_envg_t *self, double decay_t) {
    atomic_double_set(&self->decay_t, decay_t * jack_get_sample_rate(self->client));
}

void cs_envg_set_sustain_a(cs_envg_t *self, float sustain_a) {
    atomic_float_set(&self->sustain_a, sustain_a);
}

void cs_envg_set_release_t(cs_envg_t *self, double release_t) {
    atomic_double_set(&self->release_t, release_t * jack_get_sample_rate(self->client));
}

void cs_envg_set_release_a(cs_envg_t *self, float release_a) {
    atomic_float_set(&self->release_a, release_a);
}
