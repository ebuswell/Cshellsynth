#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/filter.h"
#include "cshellsynth/jclient.h"
#include "atomic-float.h"

void cs_filter_set_in(cs_filter_t *self, float in) {
    atomic_float_set(&self->in, in);
}

int cs_filter_init(cs_filter_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = jclient_init((jclient_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->in_port = jack_port_register(self->client, "in", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->in_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return -1;
    }

    self->out_port = jack_port_register(self->client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(self->out_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return -1;
    }

    atomic_float_set(&self->in, NAN);
    
    return 0;
}
