#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/synth.h"
#include "cshellsynth/jclient.h"

int cs_synth_set_freq(cs_synth_t *self, jack_default_audio_sample_t freq) {
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	self->freq = freq;
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
}

int cs_synth_init(cs_synth_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = jclient_locking_init((jclient_locking_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->freq_port = jack_port_register(self->client, "freq", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->freq_port == NULL) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return -1;
    }

    self->out_port = jack_port_register(self->client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(self->out_port == NULL) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return -1;
    }

    self->freq = NAN;

    return 0;
}
