/*
 * sequencer.h
 * 
 * Copyright 2010 Evan Buswell
 * 
 * This file is part of Cshellsynth.
 * 
 * Cshellsynth is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Cshellsynth is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Cshellsynth.  If not, see <http://www.gnu.org/licenses/>.
 */
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
int cs_seq_make_sequence(cs_seq_t *self, float offset, float length, size_t sequence_length, const float sequence[][3], bool repeat);
int cs_seq_make_sequence_ll(cs_seq_t *self, float offset, float length, float **sequence, bool repeat);
#define cs_seq_sequence(self, offset, length, sequence_length, sequence) cs_seq_make_sequence(self, offset, length, sequence_length, sequence, true);
#define cs_seq_sequence_once(self, offset, length, sequence_length, sequence) cs_seq_make_sequence(self, offset, length, sequence_length, sequence, false);

#endif
