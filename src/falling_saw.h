#ifndef FALLING_SAW_H
#define FALLING_SAW_H 1

#include <jack/jack.h>
#include "synth.h"

typedef struct cs_fsaw_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    jack_port_t *out_port;
    double offset;
} cs_fsaw_t;

#define cs_fsaw_destroy(cs_fsaw) cs_synth_destroy((cs_synth_t *) (cs_fsaw))
int cs_fsaw_init(cs_fsaw_t *self, const char *client_name, jack_options_t flags, char *server_name);

#endif
