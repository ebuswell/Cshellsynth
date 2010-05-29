#ifndef CLOCK_H
#define CLOCK_H 1

#include <jack/jack.h>
#include "jclient.h"

typedef struct cs_clock_struct {
    jack_client_t *client;
    pthread_mutex_t lock;
    jack_port_t *clock_port;
    double current;
    double step;
    double max;
} cs_clock_t;

#define cs_clock_destroy(cs_clock) jclient_locking_destroy((jclient_locking_t *) (cs_clock))
int cs_clock_init(cs_clock_t *self, const char *client_name, jack_options_t flags, char *server_name);
int cs_clock_set_meter(cs_clock_t *self, double meter);
int cs_clock_set_bpm(cs_clock_t *self, double bpm);

#endif
