#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/filter.h"
#include "cshellsynth/jclient.h"

int cs_filter_set_in(cs_filter_t *self, jack_default_audio_sample_t in) {
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	self->in = in;
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
}

int cs_filter_init(cs_filter_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = jclient_locking_init((jclient_locking_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->in_port = jack_port_register(self->client, "in", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->in_port == NULL) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return -1;
    }

    self->out_port = jack_port_register(self->client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(self->out_port == NULL) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return -1;
    }

    self->in = NAN;
    
    return 0;
}
