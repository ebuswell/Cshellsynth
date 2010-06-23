#include <jack/jack.h>
#include <pthread.h>
#include "cshellsynth/jclient.h"

int jclient_init(jclient_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    jack_status_t status;
    if(server_name == NULL) {
	self->client = jack_client_open(client_name, flags, &status);
    } else {
	self->client = jack_client_open(client_name, flags, &status, server_name);
    }
    if(self->client == NULL) {
	return status;
    } else {
	return 0;
    }
}

int jclient_destroy(jclient_t *self) {
    return jack_client_close(self->client);
}
