#ifndef CSHELLSYNTH_FILTER_H
#define CSHELLSYNTH_FILTER_H 1

#include <jack/jack.h>
#include <pthread.h>
#include <cshellsynth/jclient.h>

typedef struct cs_filter_struct {
    jack_client_t *client;
    pthread_mutex_t lock;
    jack_port_t *in_port;
    jack_default_audio_sample_t in;
    jack_port_t *out_port;
} cs_filter_t;

#define cs_filter_destroy(cs_filter) jclient_locking_destroy((jclient_locking_t *) (cs_filter))
int cs_filter_init(cs_filter_t *self, const char *client_name, jack_options_t flags, char *server_name);
int cs_filter_set_in(cs_filter_t *self, jack_default_audio_sample_t in);

#endif
