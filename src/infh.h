#ifndef INFH_H
#define INFH_H 1

#include <jack/jack.h>
#include "synth.h"

typedef struct cs_infh_struct {
    jack_client_t *client;
    pthread_mutex_t lock;
    jack_port_t *freq_port;
    jack_default_audio_sample_t freq;
    jack_port_t *out_port;
    double offset;
} cs_infh_t;

#define cs_infh_destroy(cs_infh) cs_synth_destroy((cs_synth_t *) (cs_infh))
int cs_infh_init(cs_infh_t *self, const char *client_name, jack_options_t flags, char *server_name);
#define cs_infh_set_freq(cs_infh, freq) cs_synth_set_freq((cs_synth_t *) (cs_infh), (freq))

#endif
