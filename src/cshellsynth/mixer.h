#ifndef CSHELLSYNTH_MIXER_H
#define CSHELLSYNTH_MIXER_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/jclient.h>

typedef struct cs_mix_struct {
    jack_client_t *client;
    jack_port_t *in1_port;
    atomic_float_t in1;
    jack_port_t *in2_port;
    atomic_float_t in2;
    jack_port_t *out_port;
} cs_mix_t;

#define cs_mix_destroy(cs_mix) jclient_destroy((jclient_t *) (cs_mix))
int cs_mix_init(cs_mix_t *self, const char *client_name, jack_options_t flags, char *server_name);
int cs_mix_subclass_init(cs_mix_t *self, const char *client_name, jack_options_t flags, char *server_name);
void cs_mix_set_in1(cs_mix_t *self, float in1);
void cs_mix_set_in2(cs_mix_t *self, float in2);

#endif
