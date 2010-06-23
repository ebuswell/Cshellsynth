#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/synth.h"
#include "cshellsynth/jclient.h"
#include "atomic-float.h"

void cs_synth_set_freq(cs_synth_t *self, float freq) {
    atomic_float_set(&self->freq, freq);
}

int cs_synth_init(cs_synth_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = jclient_init((jclient_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->freq_port = jack_port_register(self->client, "freq", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->freq_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return -1;
    }

    self->out_port = jack_port_register(self->client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(self->out_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return -1;
    }

    atomic_float_set(&self->freq, NAN);

    return 0;
}
