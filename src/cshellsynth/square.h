#ifndef CSHELLSYNTH_SQUARE_H
#define CSHELLSYNTH_SQUARE_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/synth.h>

typedef struct cs_square_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    atomic_float_t freq;
    jack_port_t *out_port;
    atomic_float_t amp;
    atomic_float_t offset;
    atomic_double_t duty_cycle;
    double t;
} cs_square_t;

#define cs_square_destroy(cs_square) cs_synth_destroy((cs_synth_t *) (cs_square))
int cs_square_init(cs_square_t *self, const char *client_name, jack_options_t flags, char *server_name);
#define cs_square_set_freq(cs_square, freq) cs_synth_set_freq((cs_synth_t *) (cs_square), (freq))
#define cs_square_set_offset(cs_square, offset) cs_synth_set_offset((cs_synth_t *) (cs_square), (offset))
#define cs_square_set_amp(cs_square, amp) cs_synth_set_amp((cs_synth_t *) (cs_square), (amp))
void cs_square_set_duty_cycle(cs_square_t *self, double duty_cycle);

#endif
