#include <jack/jack.h>
#include "synth.h"

int cs_synth_init(cs_synth_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    jclient_init((jclient_t *) self, client_name, flags, server_name);
    self->freq_port = jack_port_register(self->client, "freq", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutInput, 0);
    if(self->freq_port == NULL) {
	jclient_destroy(self->client);
	return -1;
    }
    self->out_port = jack_port_register(self->client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(self->out_port == NULL) {
	jclient_destroy(self->client);
	return -1;
    }
    return 0;
}
