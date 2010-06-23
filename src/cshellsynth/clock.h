#ifndef CSHELLSYNTH_CLOCK_H
#define CSHELLSYNTH_CLOCK_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/jclient.h>

typedef struct cs_clock_struct {
    jack_client_t *client;
    jack_port_t *clock_port;
    double current;
    atomic_double_t step;
    atomic_double_t max;
} cs_clock_t;

#define cs_clock_destroy(cs_clock) jclient_destroy((jclient_t *) (cs_clock))
int cs_clock_init(cs_clock_t *self, const char *client_name, jack_options_t flags, char *server_name);
void cs_clock_set_meter(cs_clock_t *self, double meter);
void cs_clock_set_bpm(cs_clock_t *self, double bpm);

#endif
