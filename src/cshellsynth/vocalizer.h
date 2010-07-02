#ifndef CSHELLSYNTH_VOCALIZER_H
#define CSHELLSYNTH_VOCALIZER_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/jclient.h>
#include <stdbool.h>

typedef struct cs_vocalizer_buffer_struct {
    int offset;
    int length;
    float *buffer;
} cs_vocalizer_buffer_t;

typedef struct cs_vocalizer_struct {
    jack_client_t *client;
    jack_port_t *ctl_port;
    jack_port_t *out_port;
    atomic_ptr_t buffer;
    bool playing;
} cs_vocalizer_t;

int cs_vocalizer_destroy(cs_vocalizer_t *self);
int cs_vocalizer_init(cs_vocalizer_t *self, const char *client_name, jack_options_t flags, char *server_name);
int cs_vocalizer_load(cs_vocalizer_t *self, char *path);

#endif
