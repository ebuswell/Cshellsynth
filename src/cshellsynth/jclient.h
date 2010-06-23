#ifndef CSHELLSYNTH_JCLIENT_H
#define CSHELLSYNTH_JCLIENT_H 1

#include <jack/jack.h>

typedef struct jclient_struct {
    jack_client_t *client;
} jclient_t;

int jclient_destroy(jclient_t *self);
int jclient_init(jclient_t *self, const char *client_name, jack_options_t flags, char *server_name);

#endif
