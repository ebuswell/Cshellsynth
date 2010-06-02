#ifndef CSHELLSYNTH_SQUARE_H
#define CSHELLSYNTH_SQUARE_H 1

#include <jack/jack.h>
#include <pthread.h>
#include <cshellsynth/synth.h>

typedef struct cs_square_struct {
    jack_client_t *client;
    pthread_mutex_t lock;
    jack_port_t *freq_port;
    jack_default_audio_sample_t freq;
    jack_port_t *out_port;
    double offset;
} cs_square_t;

#define cs_square_destroy(cs_square) cs_synth_destroy((cs_synth_t *) (cs_square))
int cs_square_init(cs_square_t *self, const char *client_name, jack_options_t flags, char *server_name);
#define cs_square_set_freq(cs_square, freq) cs_synth_set_freq((cs_synth_t *) (cs_square), (freq))

#endif