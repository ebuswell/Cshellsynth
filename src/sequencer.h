#ifndef SEQUENCER_H
#define SEQUENCER_H 1

#include <jack/jack.h>
#include <stdbool.h>
#include "controller.h"

typedef struct cs_seq_sequence_struct {
    jack_default_audio_sample_t offset;
    jack_default_audio_sample_t length;
    jack_default_audio_sample_t **seq;
    bool started;
    bool repeat;
} cs_seq_sequence_t;

typedef struct cs_seq_struct {
    jack_client_t *client;
    pthread_mutex_t lock;
    jack_port_t *ctl_port;
    jack_port_t *out_port;
    jack_port_t *clock_port;
    jack_default_audio_sample_t last; // to detect flipping
    jack_default_audio_sample_t offset;
    jack_default_audio_sample_t **current;
    jack_default_audio_sample_t out;
    bool playing;
    cs_seq_sequence_t *curr_seq;
    cs_seq_sequence_t *next_seq;
} cs_seq_t;

int cs_seq_destroy(cs_seq_t *self);
int cs_seq_init(cs_seq_t *self, const char *client_name, jack_options_t flags, char *server_name);
int cs_seq_make_sequence(cs_seq_t *self, jack_default_audio_sample_t offset, jack_default_audio_sample_t length, jack_default_audio_sample_t **sequence, bool repeat);
#define cs_seq_sequence(self, offset, length, sequence) cs_seq_make_sequence(self, offset, length, sequence, true);
#define cs_seq_sequence_once(self, offset, length, sequence) cs_seq_make_sequence(self, offset, length, sequence, false);

#endif
