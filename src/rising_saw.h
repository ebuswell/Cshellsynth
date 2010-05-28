#ifndef RISING_SAW_H
#define RISING_SAW_H 1

#include <jack/jack.h>
#include "synth.h"

typedef struct cs_rsaw_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    jack_port_t *out_port;
    double offset;
} cs_rsaw_t;

#define cs_rsaw_destroy(cs_rsaw) cs_synth_destroy((cs_synth_t *) (cs_rsaw))
int cs_rsaw_init(cs_rsaw_t *self, const char *client_name, jack_options_t flags, char *server_name);

#endif
