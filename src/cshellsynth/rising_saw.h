#ifndef CSHELLSYNTH_RISING_SAW_H
#define CSHELLSYNTH_RISING_SAW_H 1

#include <jack/jack.h>
#include <pthread.h>
#include <cshellsynth/synth.h>

typedef struct cs_rsaw_struct {
    jack_client_t *client;
    pthread_mutex_t lock;
    jack_port_t *freq_port;
    jack_default_audio_sample_t freq;
    jack_port_t *out_port;
    double offset;
} cs_rsaw_t;

#define cs_rsaw_destroy(cs_rsaw) cs_synth_destroy((cs_synth_t *) (cs_rsaw))
int cs_rsaw_init(cs_rsaw_t *self, const char *client_name, jack_options_t flags, char *server_name);
#define cs_rsaw_set_freq(cs_rsaw, freq) cs_synth_set_freq((cs_synth_t *) (cs_rsaw), (freq))

#endif
