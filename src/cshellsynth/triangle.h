#ifndef CSHELLSYNTH_TRIANGLE_H
#define CSHELLSYNTH_TRIANGLE_H 1

#include <jack/jack.h>
#include <pthread.h>
#include <cshellsynth/synth.h>

typedef struct cs_triangle_struct {
    jack_client_t *client;
    pthread_mutex_t lock;
    jack_port_t *freq_port;
    jack_default_audio_sample_t freq;
    jack_port_t *out_port;
    double offset;
} cs_triangle_t;

#define cs_triangle_destroy(cs_triangle) cs_synth_destroy((cs_synth_t *) (cs_triangle))
int cs_triangle_init(cs_triangle_t *self, const char *client_name, jack_options_t flags, char *server_name);
#define cs_triangle_set_freq(cs_triangle, freq) cs_synth_set_freq((cs_synth_t *) (cs_triangle), (freq))

#endif
