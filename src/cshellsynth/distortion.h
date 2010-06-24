#ifndef CSHELLSYNTH_DISTORTION_H
#define CSHELLSYNTH_DISTORTION_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/filter.h>

typedef struct cs_distort_struct {
    jack_client_t *client;
    jack_port_t *in_port;
    atomic_float_t in;
    jack_port_t *out_port;
    jack_port_t *gain_port;
    atomic_float_t gain;
    atomic_float_t sharpness;
} cs_distort_t;

#define cs_distort_destroy(cs_distort) cs_filter_destroy((cs_filter_t *) (cs_distort))
int cs_distort_init(cs_distort_t *self, const char *client_name, jack_options_t flags, char *server_name);
#define cs_distort_set_in(self, in) cs_filter_set_in(self, in)
void cs_distort_set_gain(cs_distort_t *self, float gain);
void cs_distort_set_sharpness(cs_distort_t *self, float sharpness);

#endif
