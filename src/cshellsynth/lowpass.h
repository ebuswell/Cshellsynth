#ifndef CSHELLSYNTH_LOWPASS_H
#define CSHELLSYNTH_LOWPASS_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/filter.h>

typedef struct cs_lowpass_struct {
    jack_client_t *client;
    jack_port_t *in_port;
    atomic_float_t in;
    jack_port_t *out_port;
    jack_port_t *freq_port;
    atomic_float_t freq;
    double last_out;
} cs_lowpass_t;

#define cs_lowpass_destroy(cs_lowpass) cs_filter_destroy((cs_filter_t *) (cs_lowpass))
int cs_lowpass_init(cs_lowpass_t *self, const char *client_name, jack_options_t flags, char *server_name);
#define cs_lowpass_set_in(self, in) cs_filter_set_in(self, in)
void cs_lowpass_set_freq(cs_lowpass_t *self, float freq);

#endif
