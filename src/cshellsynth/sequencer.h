#ifndef CSHELLSYNTH_SEQUENCER_H
#define CSHELLSYNTH_SEQUENCER_H 1

#include <jack/jack.h>
#include <stdbool.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/controller.h>

typedef struct cs_seq_sequence_struct {
    float offset;
    float length;
    float **seq;
    bool started;
    bool repeat;
} cs_seq_sequence_t;

typedef struct cs_seq_struct {
    jack_client_t *client;
    jack_port_t *ctl_port;
    jack_port_t *out_port;
    jack_port_t *clock_port;
    float last; // to detect flipping
    float offset;
    float **current;
    float out;
    bool playing;
    cs_seq_sequence_t *curr_seq;
    atomic_ptr_t next_seq; // cs_seq_sequence_t *
} cs_seq_t;

int cs_seq_destroy(cs_seq_t *self);
int cs_seq_init(cs_seq_t *self, const char *client_name, jack_options_t flags, char *server_name);
void cs_seq_make_sequence(cs_seq_t *self, float offset, float length, size_t sequence_length, const float sequence[][3], bool repeat);
void cs_seq_make_sequence_ll(cs_seq_t *self, float offset, float length, float **sequence, bool repeat);
#define cs_seq_sequence(self, offset, length, sequence_length, sequence) cs_seq_make_sequence(self, offset, length, sequence_length, sequence, true);
#define cs_seq_sequence_once(self, offset, length, sequence_length, sequence) cs_seq_make_sequence(self, offset, length, sequence_length, sequence, false);

#endif
