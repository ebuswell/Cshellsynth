#ifndef TRIANGLE_H
#define TRIANGLE_H 1

#include <jack/jack.h>
#include "synth.h"

typedef struct cs_triangle_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    jack_port_t *out_port;
    double offset;
} cs_triangle_t;

#define cs_triangle_destroy(cs_triangle) cs_synth_destroy((cs_synth_t *) (cs_triangle))
int cs_triangle_init(cs_triangle_t *self, const char *client_name, jack_options_t flags, char *server_name);

#endif
