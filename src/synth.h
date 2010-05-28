#ifndef SYNTH_H
#define SYNTH_H 1

#include <jack/jack.h>

typedef struct cs_synth_struct {
    jack_client_t *client;
    pthread_mutex_t lock;
    jack_port_t *freq_port;
    jack_default_audio_sample_t freq;
    jack_port_t *out_port;
} cs_synth_t;

#define cs_synth_destroy(cs_synth) jclient_locking_destroy((jclient_locking_t *) (cs_synth))
int cs_synth_init(cs_synth_t *self, const char *client_name, jack_options_t flags, char *server_name);
int cs_synth_set_freq(cs_synth_t *self, jack_default_audio_sample_t freq);

#endif
