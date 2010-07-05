#ifndef CSHELLSYNTH_SYNTH_H
#define CSHELLSYNTH_SYNTH_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/jclient.h>

typedef struct cs_synth_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    atomic_float_t freq;
    jack_port_t *out_port;
    atomic_float_t amp;
    atomic_float_t offset;
} cs_synth_t;

#define cs_synth_destroy(cs_synth) jclient_destroy((jclient_t *) (cs_synth))
int cs_synth_init(cs_synth_t *self, const char *client_name, jack_options_t flags, char *server_name);
void cs_synth_set_freq(cs_synth_t *self, float freq);
void cs_synth_set_offset(cs_synth_t *self, float offset);
void cs_synth_set_amp(cs_synth_t *self, float amp);

#endif
