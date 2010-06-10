#ifndef CSHELLSYNTH_LIN2EXP_H
#define CSHELLSYNTH_LIN2EXP_H 1

#include <jack/jack.h>
#include <pthread.h>
#include <cshellsynth/filter.h>

typedef struct cs_lin2exp_struct {
    jack_client_t *client;
    pthread_mutex_t lock;
    jack_port_t *in_port;
    jack_default_audio_sample_t in;
    jack_port_t *out_port;
    jack_default_audio_sample_t zero;
} cs_lin2exp_t;

#define cs_lin2exp_destroy(cs_lin2exp) cs_filter_destroy((cs_filter_t *) (cs_lin2exp))
int cs_lin2exp_init(cs_lin2exp_t *self, const char *client_name, jack_options_t flags, char *server_name);
#define cs_lin2exp_set_in(self, in) cs_filter_set_in((cs_filter_t *) (self), in);
int cs_lin2exp_set_zero(cs_lin2exp_t *self, jack_default_audio_sample_t zero);

#endif
