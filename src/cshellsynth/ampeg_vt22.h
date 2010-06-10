#ifndef CSHELLSYNTH_AMPEG_VT22_H
#define CSHELLSYNTH_AMPEG_VT22_H 1

#include <jack/jack.h>
#include <pthread.h>
#include <cshellsynth/filter.h>

typedef struct cs_ampeg_vt22_struct {
    jack_client_t *client;
    pthread_mutex_t lock;
    jack_port_t *in_port;
    jack_default_audio_sample_t in;
    jack_port_t *out_port;
    jack_default_audio_sample_t gain;
} cs_ampeg_vt22_t;

#define cs_ampeg_vt22_destroy(cs_ampeg_vt22) cs_filter_destroy((cs_filter_t *) (cs_ampeg_vt22))
int cs_ampeg_vt22_init(cs_ampeg_vt22_t *self, const char *client_name, jack_options_t flags, char *server_name);
int cs_ampeg_vt22_set_in(cs_ampeg_vt22_t *self, jack_default_audio_sample_t in);
int cs_ampeg_vt22_set_gain(cs_ampeg_vt22_t *self, jack_default_audio_sample_t gain);

#endif
