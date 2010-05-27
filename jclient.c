#include <jack/jack.h>
#include <pthread.h>
#include "jclient.h"

int jclient_init(jclient_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    jack_status_t status;
    if(server_name == NULL) {
	self->client = jack_client_open(client_name, flags, &status);
    } else {
	self->client = jack_client_open(client_name, flags, &status, server_name);
    }
    if(cself->client == NULL) {
	return status;
    } else {
	return 0;
    }
}

int jclient_locking_init(jclient_locking_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = jclient_init((jclient_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }
    pthread_mutexattr_t attr;
    r = pthread_mutexattr_init(&attr);
    if(r != 0) {
	return r;
    }
    r = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    if(r != 0) {
	return r;
    }
    r = pthread_mutex_init(&self->lock, &attr);
    if(r != 0) {
	return r;
    }
    r = pthread_mutexattr_destroy(&attr);
    if(r != 0) {
	return r;
    }
    return 0;
}

int jclient_destroy(jclient_t *self) {
    return jack_client_close(self->client);
}

int jclient_locking_destroy(jclient_locking_t *self) {
    int r = pthread_mutex_destroy(&self->lock);
    if(r != 0) {
	return r;
    }
    return jclient_destroy((jclient_t *) self);
}
