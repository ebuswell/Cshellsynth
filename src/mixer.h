#ifndef MIXER_H
#define MIXER_H 1

#include <jack/jack.h>
#include "jclient.h"

typedef struct cs_mix_struct {
    jack_client_t *client;
    pthread_mutex_t lock;
    jack_port_t *in1_port;
    jack_default_audio_sample_t in1;
    jack_port_t *in2_port;
    jack_default_audio_sample_t in2;
    jack_port_t *out_port;
} cs_mix_t;

#define cs_mix_destroy(cs_mix) jclient_locking_destroy((jclient_locking_t *) (cs_mix))
int cs_mix_init(cs_mix_t *self, const char *client_name, jack_options_t flags, char *server_name);
int cs_mix_set_in1(cs_mix_t *self, jack_default_audio_sample_t in1);
int cs_mix_set_in2(cs_mix_t *self, jack_default_audio_sample_t in2);

#endif
