#ifndef CSHELLSYNTH_AMPEG_VT22_H
#define CSHELLSYNTH_AMPEG_VT22_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/filter.h>

typedef struct cs_ampeg_vt22_struct {
    jack_client_t *client;
    jack_port_t *in_port;
    atomic_float_t in;
    jack_port_t *out_port;
    atomic_float_t gain;
} cs_ampeg_vt22_t;

#define cs_ampeg_vt22_destroy(cs_ampeg_vt22) cs_filter_destroy((cs_filter_t *) (cs_ampeg_vt22))
int cs_ampeg_vt22_init(cs_ampeg_vt22_t *self, const char *client_name, jack_options_t flags, char *server_name);
void cs_ampeg_vt22_set_in(cs_ampeg_vt22_t *self, float in);
void cs_ampeg_vt22_set_gain(cs_ampeg_vt22_t *self, float gain);

#endif
