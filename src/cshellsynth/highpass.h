#ifndef CSHELLSYNTH_HIGHPASS_H
#define CSHELLSYNTH_HIGHPASS_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/filter.h>

typedef struct cs_highpass_struct {
    jack_client_t *client;
    jack_port_t *in_port;
    atomic_float_t in;
    jack_port_t *out_port;
    jack_port_t *freq_port;
    atomic_float_t freq;
    double last_out;
    float last_in;
} cs_highpass_t;

#define cs_highpass_destroy(cs_highpass) cs_filter_destroy((cs_filter_t *) (cs_highpass))
int cs_highpass_init(cs_highpass_t *self, const char *client_name, jack_options_t flags, char *server_name);
#define cs_highpass_set_in(self, in) cs_filter_set_in(self, in)
void cs_highpass_set_freq(cs_highpass_t *self, float freq);

#endif
