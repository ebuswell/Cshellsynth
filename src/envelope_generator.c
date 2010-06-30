#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/envelope_generator.h"
#include "cshellsynth/jclient.h"
#include "atomic-float.h"
#include "atomic-double.h"

#define ONE_ADJUST 1.0451657053636841
/* (1.0/(1.0 - exp(-M_PI)) */

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
    double decay_t = atomic_double_read(&self->decay_t);
    float sustain_a = atomic_float_read(&self->sustain_a);
    double release_t = atomic_double_read(&self->release_t);
    int linear = atomic_read(&self->linear);
    int i;
    for(i = 0; i < nframes; i++) {
	float ctl = ctl_buffer[i];
	if(isnanf(ctl)) {
	    ctl = 0.0f;
	}
	if(ctl > 0.0f) {
	    // attack event
	    self->offset = 0.0;
	    self->state = ATTACK;
	    self->start_a = self->last_a;
	} else if(ctl < 0.0f) {
	    // release event
	    self->offset = 0.0;
	    self->state = RELEASE;
	    self->start_a = self->last_a;
	}
	switch(self->state) {
	case ATTACK:
	    if(self->offset < attack_t) {
		if(linear) {
		    out_buffer[i] = self->last_a = ((float) ((self->offset / attack_t) * ((double) (1.0f - self->start_a)))) + self->start_a;
		    self->offset += 1.0;
		    break;
		} else {
		    out_buffer[i] = self->last_a = self->start_a + ((float) ((ONE_ADJUST - ((double) self->start_a)) * (1.0 - exp(-M_PI * self->offset / attack_t))));
		    if(self->last_a < 1.0f) {
			self->offset += 1.0;
			break;
		    } else {
			self->state = DECAY;
			self->offset = 0;
			// fall through
		    }
		}
	    } else {
		self->state = DECAY;
		self->offset -= attack_t;
		// fall through
	    }
	case DECAY:
	    if(linear) {
		if(self->offset < decay_t) {
		    out_buffer[i] = self->last_a = ((float) (1.0 - (self->offset / decay_t))) * (1.0f - sustain_a) + sustain_a;
		    self->offset += 1.0;
		    break;
		} else {
		    self->state = SUSTAIN;
		    // fall through
		}
	    } else {
		if(decay_t != 0.0) {
		    float adjust = (((double) (1.0f - sustain_a))*exp(-M_PI * self->offset / decay_t));
		    if(adjust == 0.0f) {
			self->state = SUSTAIN;
			// fall through
		    } else {
			out_buffer[i] = self->last_a = sustain_a + adjust;
			self->offset += 1.0;
			break;
		    }
		} else {
		    self->state = SUSTAIN;
		    // fall through
		}
	    }
	case SUSTAIN:
	    out_buffer[i] = self->last_a = sustain_a;
	    break;
	case RELEASE:
	    if(linear) {
		if(self->offset < release_t) {
		    out_buffer[i] = self->last_a = ((1.0 - (self->offset / release_t)) * ((double) self->start_a));
		    self->offset += 1.0;
		    break;
		} else {
		    self->state = FINISHED;
		    // fall through
		}
	    } else {
		if(release_t != 0.0) {
		    out_buffer[i] = self->last_a = (((double) self->start_a) * exp(-M_PI * self->offset / release_t));
		    if(self->last_a == 0.0f) {
			self->state = FINISHED;
			break;
		    } else {
			self->offset += 1.0;
			break;
		    }
		} else {
		    self->state = FINISHED;
		    // fall through
		}
	    }
	case FINISHED:
	default:
	    out_buffer[i] = self->last_a = 0.0f;
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
    atomic_double_set(&self->decay_t, 0.0);
    atomic_float_set(&self->sustain_a, 1.0f);
    atomic_double_set(&self->release_t, 0.0);
    atomic_set(&self->linear, 0);
    self->state = FINISHED;

    r = jack_set_process_callback(self->client, cs_envg_process, self);
    if(r != 0) {
	jclient_destroy((jclient_t *) self);
	return r;
    }
    self->offset = 0.0;
    self->start_a = 0.0f;
    self->last_a = 0.0f;
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

void cs_envg_set_decay_t(cs_envg_t *self, double decay_t) {
    atomic_double_set(&self->decay_t, decay_t * jack_get_sample_rate(self->client));
}

void cs_envg_set_sustain_a(cs_envg_t *self, float sustain_a) {
    atomic_float_set(&self->sustain_a, sustain_a);
}

void cs_envg_set_release_t(cs_envg_t *self, double release_t) {
    atomic_double_set(&self->release_t, release_t * jack_get_sample_rate(self->client));
}
