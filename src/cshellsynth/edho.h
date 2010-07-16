#ifndef CSHELLSYNTH_EDHO_H
#define CSHELLSYNTH_EDHO_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/synth.h>

typedef struct cs_edho_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    atomic_float_t freq;
    jack_port_t *out_port;
    atomic_float_t amp;
    atomic_float_t offset;
    double t;
    jack_port_t *bright_port;
    atomic_float_t bright;
    atomic_t scale;
} cs_edho_t;

#define cs_edho_destroy(cs_edho) cs_synth_destroy((cs_synth_t *) (cs_edho))
int cs_edho_init(cs_edho_t *self, const char *client_name, jack_options_t flags, char *server_name);
#define cs_edho_set_freq(cs_edho, freq) cs_synth_set_freq((cs_synth_t *) (cs_edho), (freq))
#define cs_edho_set_offset(cs_edho, offset) cs_synth_set_offset((cs_synth_t *) (cs_edho), (offset))
#define cs_edho_set_amp(cs_edho, amp) cs_synth_set_amp((cs_synth_t *) (cs_edho), (amp))
void cs_edho_set_bright(cs_edho_t *self, float bright);

#endif
