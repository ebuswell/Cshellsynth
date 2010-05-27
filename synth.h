#ifndef SYNTH_H
#define SYNTH_H 1

#include <jack/jack.h>

typedef struct cs_synth_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    jack_port_t *out_port;
} cs_synth_t;

#define cs_synth_destroy(cs_synth) jclient_destroy((jclient_t *) (cs_synth))
int cs_synth_init(cs_synth_t *self, const char *client_name, jack_options_t flags, char *server_name);

#endif
