#ifndef CSHELLSYNTH_SAMPLER_H
#define CSHELLSYNTH_SAMPLER_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/jclient.h>
#include <stdbool.h>

typedef struct cs_sampler_struct {
    jack_client_t *client;
    jack_port_t *ctl_port;
    jack_port_t *outL_port;
    jack_port_t *outR_port;
    atomic_ptr_t sf;
    bool playing;
} cs_sampler_t;

int cs_sampler_destroy(cs_sampler_t *self);
int cs_sampler_init(cs_sampler_t *self, const char *client_name, jack_options_t flags, char *server_name);
int cs_sampler_load(cs_sampler_t *self, char *path);

#endif
