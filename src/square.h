#ifndef SQUARE_H
#define SQUARE_H 1

#include <jack/jack.h>
#include "synth.h"

typedef struct cs_square_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    jack_port_t *out_port;
    double offset;
} cs_square_t;

#define cs_square_destroy(cs_square) cs_synth_destroy((cs_synth_t *) (cs_square))
int cs_square_init(cs_square_t *self, const char *client_name, jack_options_t flags, char *server_name);

#endif
