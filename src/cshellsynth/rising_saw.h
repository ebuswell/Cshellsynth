#ifndef CSHELLSYNTH_RISING_SAW_H
#define CSHELLSYNTH_RISING_SAW_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/synth.h>

typedef struct cs_rsaw_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    atomic_float_t freq;
    jack_port_t *out_port;
    atomic_float_t amp;
    atomic_float_t offset;
    double t;
} cs_rsaw_t;

#define cs_rsaw_destroy(cs_rsaw) cs_synth_destroy((cs_synth_t *) (cs_rsaw))
int cs_rsaw_init(cs_rsaw_t *self, const char *client_name, jack_options_t flags, char *server_name);
#define cs_rsaw_set_freq(cs_rsaw, freq) cs_synth_set_freq((cs_synth_t *) (cs_rsaw), (freq))
#define cs_rsaw_set_offset(cs_rsaw, offset) cs_synth_set_offset((cs_synth_t *) (cs_rsaw), (offset))
#define cs_rsaw_set_amp(cs_rsaw, amp) cs_synth_set_amp((cs_synth_t *) (cs_rsaw), (amp))

#endif
