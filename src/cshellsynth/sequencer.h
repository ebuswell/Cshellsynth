/** @file sequencer.h
 *
 * Sequencer
 *
 * Ruby version: @c Controllers::Sequencer
 */
/*
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

/**
 * A particular sequence
 */
typedef struct cs_seq_sequence_struct {
    float offset;
    float length;
    float **seq;
    bool started;
    bool repeat;
} cs_seq_sequence_t;

/** @file */

/**
 * Sequencer
 *
 * Ruby version: @c Controllers::Sequencer
 *
 * See @ref cs_ctlr_t
 */
typedef struct cs_seq_struct {
    jack_client_t *client;
    jack_port_t *ctl_port;
    jack_port_t *out_port;
    jack_port_t *clock_port; /** Clock input */
    float last; /** Last clock value, to detect flipping */
    float offset; /** How far this sequencer is offset from clock values */
    float **current; /** Current sequence (working copy) */
    float out; /** Current output value */
    bool playing; /** Whether we're currently playing a note or stopped */
    cs_seq_sequence_t *curr_seq; /** Current sequence */
    atomic_ptr_t next_seq; /** Next sequence (cs_seq_sequence_t *) */
} cs_seq_t;

/**
 * Destroy sequencer
 *
 * See @ref cs_ctlr_destroy
 */
int cs_seq_destroy(cs_seq_t *self);

/**
 * Initialize sequencer
 *
 * See @ref cs_ctlr_init
 */
int cs_seq_init(cs_seq_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * Make sequence
 *
 * Creates a sequence, calling @ref cs_seq_make_sequence_ll internally.
 *
 * @param offset what this sequencer considers time zero.
 * @param length the duration of this sequence.
 * @param sequence_length the length of the sequence argument.
 * @param sequence a sequence of triples, <tt>[start, end, value]</tt> indicating the
 * sequence to play.
 * @param repeat whether the sequence immediately repeats when finished or stops.  If
 * there is a sequence queued, this value is ignored.
 */
int cs_seq_make_sequence(cs_seq_t *self, float offset, float length, size_t sequence_length, const float sequence[][3], bool repeat);

/**
 * Internal version of @ref cs_seq_make_sequence which uses null-terminated pointer arrays
 * instead of multidimensional arrays.
 */
int cs_seq_make_sequence_ll(cs_seq_t *self, float offset, float length, float **sequence, bool repeat);

/**
 * Convenience method to call @ref cs_seq_make_sequence with <tt>repeat = true</tt>.
 *
 * Ruby version: @c sequence
 */
#define cs_seq_sequence(self, offset, length, sequence_length, sequence) cs_seq_make_sequence(self, offset, length, sequence_length, sequence, true);

/**
 * Convenience method to call @ref cs_seq_make_sequence with <tt>repeat = true</tt>.
 *
 * Ruby version: @c sequence_once
 */
#define cs_seq_sequence_once(self, offset, length, sequence_length, sequence) cs_seq_make_sequence(self, offset, length, sequence_length, sequence, false);

#endif
