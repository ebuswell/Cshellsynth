#ifndef CSHELLSYNTH_BANDPASS_H
#define CSHELLSYNTH_BANDPASS_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/filter.h>

typedef struct cs_bandpass_struct {
    jack_client_t *client;
    jack_port_t *in_port;
    atomic_float_t in;
    jack_port_t *out_port;
    jack_port_t *freq_port;
    atomic_float_t freq;
    double last_out;
    double out_accumulator;
    atomic_double_t Q;
} cs_bandpass_t;

#define cs_bandpass_destroy(cs_bandpass) cs_filter_destroy((cs_filter_t *) (cs_bandpass))
int cs_bandpass_init(cs_bandpass_t *self, const char *client_name, jack_options_t flags, char *server_name);
#define cs_bandpass_set_in(self, in) cs_filter_set_in(self, in)
void cs_bandpass_set_freq(cs_bandpass_t *self, float freq);
void cs_bandpass_set_Q(cs_bandpass_t *self, double Q);

#endif
