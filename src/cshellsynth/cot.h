#ifndef CSHELLSYNTH_COT_H
#define CSHELLSYNTH_COT_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/synth.h>

typedef struct cs_cot_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    atomic_float_t freq;
    jack_port_t *out_port;
    double ft_1;
} cs_cot_t;

#define cs_cot_destroy(cs_cot) cs_synth_destroy((cs_synth_t *) (cs_cot))
int cs_cot_init(cs_cot_t *self, const char *client_name, jack_options_t flags, char *server_name);
#define cs_cot_set_freq(cs_cot, freq) cs_synth_set_freq((cs_synth_t *) (cs_cot), (freq))

#endif
