#ifndef CSHELLSYNTH_JCLIENT_H
#define CSHELLSYNTH_JCLIENT_H 1

#include <jack/jack.h>
#include <pthread.h>

typedef struct jclient_struct {
    jack_client_t *client;
} jclient_t;

typedef struct jclient_locking_struct {
    jack_client_t *client;
    pthread_mutex_t lock;
} jclient_locking_t;

int jclient_destroy(jclient_t *self);
int jclient_init(jclient_t *self, const char *client_name, jack_options_t flags, char *server_name);

int jclient_locking_destroy(jclient_locking_t *self);
int jclient_locking_init(jclient_locking_t *self, const char *client_name, jack_options_t flags, char *server_name);

#endif
