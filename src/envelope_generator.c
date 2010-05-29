#include <jack/jack.h>
#include <math.h>
#include "envelope_generator.h"
#include "jclient.h"

static int cs_envg_process(jack_nframes_t nframes, void *arg) {
    cs_envg_t *self = (cs_envg_t *) arg;
    jack_default_audio_sample_t *ctl_buffer = (jack_default_audio_sample_t *) jack_port_get_buffer(self->ctl_port, nframes);
    if(ctl_buffer == NULL) {
	return -1;
    }
    jack_default_audio_sample_t *out_buffer = (jack_default_audio_sample_t *) jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	int i;
	for(i = 0; i < nframes; i++) {
	    jack_default_audio_sample_t ctl = ctl_buffer[i];
	    if(isnanf(ctl)) {
		ctl = 0.0f;
	    }
	    if(ctl > 0.0f) {
		// attack event
		self->offset = 0.0;
		self->state = ATTACK;
	    } else if(ctl < 0.0f) {
		// release event
		self->offset = 0.0;
		self->state = RELEASE;
	    }
	    switch(self->state) {
	    case ATTACK:
		if(self->offset < self->attack_t) {
		    out_buffer[i] = ((jack_default_audio_sample_t) self->offset / self->attack_t);
		    self->offset += 1.0;
		    break;
		} else {
		    self->state = DECAY;
		    self->offset -= self->attack_t;
		    // fall through
		}
	    case DECAY:
		if(self->offset < self->decay_t) {
		    out_buffer[i] = ((jack_default_audio_sample_t) (1.0 - (self->offset / self->decay_t))) * (1.0f - self->sustain_a) + self->sustain_a;
		    self->offset += 1.0;
		    break;
		} else {
		    self->state = SUSTAIN;
		    // fall through
		}
	    case SUSTAIN:
		out_buffer[i] = self->sustain_a;
		break;
	    case RELEASE:
		if(self->offset < self->release_t) {
		    out_buffer[i] = ((jack_default_audio_sample_t) (1.0 - (self->offset / self->release_t))) * self->sustain_a;
		    self->offset += 1.0;
		    break;
		} else {
		    self->state = FINISHED;
		    // fall through
		}
	    case FINISHED:
	    default:
		out_buffer[i] = 0.0f;
	    }
	}
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

int cs_envg_init(cs_envg_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = jclient_locking_init((jclient_locking_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->ctl_port = jack_port_register(self->client, "ctl", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->ctl_port == NULL) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return -1;
    }

    self->out_port = jack_port_register(self->client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(self->out_port == NULL) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return -1;
    }

    self->attack_t = 0.0;
    self->decay_t = 0.0;
    self->sustain_a = 1.0f;
    self->release_t = 0.0;
    self->offset = 0.0;

    r = jack_set_process_callback(self->client, cs_envg_process, self);
    if(r != 0) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return r;
    }
    self->offset = 0.0;
    r = jack_activate(self->client);
    if(r != 0) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return r;
    }
    return 0;
}

int cs_envg_set_attack_t(cs_envg_t *self, double attack_t) {
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	self->attack_t = attack_t * ((double) jack_get_sample_rate(self->client));
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

int cs_envg_set_decay_t(cs_envg_t *self, double decay_t) {
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	self->decay_t = decay_t * ((double) jack_get_sample_rate(self->client));
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

int cs_envg_set_sustain_a(cs_envg_t *self, jack_default_audio_sample_t sustain_a) {
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	self->sustain_a = sustain_a;
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

int cs_envg_set_release_t(cs_envg_t *self, double release_t) {
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	self->release_t = release_t * ((double) jack_get_sample_rate(self->client));
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}
