#ifndef MODULATOR_H
#define MODULATOR_H 1

#include <jack/jack.h>
#include "jclient.h"

typedef struct cs_modu_struct {
    jack_client_t *client;
    jack_port_t *in1_port;
    jack_port_t *in2_port;
    jack_port_t *out_port;
} cs_modu_t;

#define cs_modu_destroy(cs_modu) jclient_destroy((jclient_t *) (cs_modu))
int cs_modu_init(cs_modu_t *self, const char *client_name, jack_options_t flags, char *server_name);

#endif
