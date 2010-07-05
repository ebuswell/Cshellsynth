#ifndef CSHELLNOISE_NOISE_H
#define CSHELLNOISE_NOISE_H 1

#include <jack/jack.h>
#include <cshellsynth/jclient.h>
#include <cshellsynth/atomic-types.h>

typedef struct cs_noise_struct {
    jack_client_t *client;
    jack_port_t *out_port;
    atomic_float_t amp;
    atomic_float_t offset;
    atomic_t kind;
    float state[3];
} cs_noise_t;

#define cs_noise_destroy(cs_noise) jclient_destroy((jclient_t *) (cs_noise))
int cs_noise_init(cs_noise_t *self, const char *client_name, jack_options_t flags, char *server_name);
void cs_noise_set_kind(cs_noise_t *self, int kind);

#define CS_WHITE 1
#define CS_PINK 2
#define CS_RED 3

#endif
