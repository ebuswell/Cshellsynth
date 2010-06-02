#include <jack/jack.h>
#include "cshellsynth/controller.h"
#include "cshellsynth/jclient.h"

int cs_ctlr_init(cs_ctlr_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = jclient_locking_init((jclient_locking_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->ctl_port = jack_port_register(self->client, "ctl", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(self->ctl_port == NULL) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return -1;
    }

    self->out_port = jack_port_register(self->client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(self->out_port == NULL) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return -1;
    }

    return 0;
}
