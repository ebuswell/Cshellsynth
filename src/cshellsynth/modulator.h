#ifndef CSHELLSYNTH_MODULATOR_H
#define CSHELLSYNTH_MODULATOR_H 1

#include <jack/jack.h>
#include <pthread.h>
#include <cshellsynth/mixer.h>

typedef struct cs_modu_struct {
    jack_client_t *client;
    pthread_mutex_t lock;
    jack_port_t *in1_port;
    jack_default_audio_sample_t in1;
    jack_port_t *in2_port;
    jack_default_audio_sample_t in2;
    jack_port_t *out_port;
} cs_modu_t;

#define cs_modu_destroy(cs_modu) cs_mix_destroy((cs_mix_t *) (cs_modu))
int cs_modu_init(cs_modu_t *self, const char *client_name, jack_options_t flags, char *server_name);
#define cs_modu_set_in1(self, in1) cs_mix_set_in1((cs_mix_t *) (self), in1);
#define cs_modu_set_in2(self, in2) cs_mix_set_in2((cs_mix_t *) (self), in2);

#endif
