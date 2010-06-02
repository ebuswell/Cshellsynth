#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/clock.h"
#include "cshellsynth/jclient.h"

int cs_clock_set_meter(cs_clock_t *self, double meter) {
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	self->max = meter;
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
}

int cs_clock_set_bpm(cs_clock_t *self, double bpm) {
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	self->step = bpm / (60.0 * ((double) jack_get_sample_rate(self->client)));
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
}

static int cs_clock_process(jack_nframes_t nframes, void *arg) {
    cs_clock_t *self = (cs_clock_t *) arg;
    jack_default_audio_sample_t *clock_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(self->clock_port, nframes);
    if(clock_buffer == NULL) {
	return -1;
    }
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	int i;
	for(i = 0; i < nframes; i++) {
	    clock_buffer[i] = (jack_default_audio_sample_t) self->current;
	    self->current += self->step;
	    while(self->current >= self->max) {
		self->current -= self->max;
	    }
	}
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

int cs_clock_init(cs_clock_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = jclient_locking_init((jclient_locking_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->clock_port = jack_port_register(self->client, "clock", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(self->clock_port == NULL) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return -1;
    }

    self->current = 0.0;
    self->max = 4.0;
    self->step = 120.0 / (60.0 * ((double) jack_get_sample_rate(self->client)));

    r = jack_set_process_callback(self->client, cs_clock_process, self);
    if(r != 0) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return r;
    }

    r = jack_activate(self->client);
    if(r != 0) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return r;
    }

    return 0;
}
