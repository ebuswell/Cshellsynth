#ifndef CSHELLSYNTH_SINE_H
#define CSHELLSYNTH_SINE_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/synth.h>
#include <stdbool.h>

typedef struct cs_sine_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    atomic_float_t freq;
    jack_port_t *out_port;
    double offset;
} cs_sine_t;

#define cs_sine_destroy(cs_sine) cs_synth_destroy((cs_synth_t *) (cs_sine))
int cs_sine_init(cs_sine_t *self, const char *client_name, jack_options_t flags, char *server_name);
#define cs_sine_set_freq(cs_sine, freq) cs_synth_set_freq((cs_synth_t *) (cs_sine), (freq))

#endif
