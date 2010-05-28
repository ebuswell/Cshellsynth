#include <jack/jack.h>
#include "modulator.h"
#include "jclient.h"

static int cs_modu_process(jack_nframes_t nframes, void *arg) {
    cs_modu_t *self = (cs_modu_t *) arg;
    jack_default_audio_sample_t *in1_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(self->in1_port, nframes);
    if(in1_buffer == NULL) {
	return -1;
    }
    jack_default_audio_sample_t *in2_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(self->in2_port, nframes);
    if(in2_buffer == NULL) {
	return -1;
    }
    jack_default_audio_sample_t *out_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    int i;
    for(i = 0; i < nframes; i++) {
	out_buffer[i] = in1_buffer[i] * in2_buffer[i];
    }
    return 0;
}

int cs_modu_init(cs_modu_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = jclient_init((jclient_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->in1_port = jack_port_register(self->client, "in1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->in1_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return -1;
    }

    self->in2_port = jack_port_register(self->client, "in2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->in2_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return -1;
    }

    self->out_port = jack_port_register(self->client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(self->out_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return -1;
    }

    r = jack_set_process_callback(self->client, cs_modu_process, self);
    if(r != 0) {
	jclient_destroy((jclient_t *) self);
	return r;
    }

    return 0;
}
