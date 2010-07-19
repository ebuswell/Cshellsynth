/** @file cot.h
 *
 * Cotangent Wave Synth
 *
 * Ruby version: @c Synths::Cotangent
 *
 * This produces a wave of the form <tt>cot(wt/2)/2</tt>
 *
 * This is useful because
 *
 * @verbatim
 *
 * inf
 *  E sin(nwt) = cot(wt/2)/2
 * n=1
 *
 * @endverbatim
 *
 * Note that cotangent has an infinite peak (here reduced to HUGE), so you will want to
 * run this through Distortion (@ref distort.h) to limit it.
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
#ifndef CSHELLSYNTH_COT_H
#define CSHELLSYNTH_COT_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/synth.h>

/**
 * Cotangent Wave Synth
 *
 * Ruby version: @c Synths::Cotangent
 *
 * See @ref cs_synth_t
 */
typedef struct cs_cot_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    atomic_float_t freq;
    jack_port_t *out_port;
    atomic_float_t amp;
    atomic_float_t offset;
    double t;
} cs_cot_t;

/**
 * Destroy cot synth
 *
 * See @ref cs_synth_destroy
 */
#define cs_cot_destroy(cs_cot) cs_synth_destroy((cs_synth_t *) (cs_cot))

/**
 * Initialize cot synth
 *
 * See @ref cs_synth_destroy
 */
int cs_cot_init(cs_cot_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * @ref cs_synth_set_freq
 */
#define cs_cot_set_freq(cs_cot, freq) cs_synth_set_freq((cs_synth_t *) (cs_cot), (freq))

/**
 * @ref cs_synth_set_offset
 */
#define cs_cot_set_offset(cs_cot, offset) cs_synth_set_offset((cs_synth_t *) (cs_cot), (offset))

/**
 * @ref cs_synth_set_amp
 */
#define cs_cot_set_amp(cs_cot, amp) cs_synth_set_amp((cs_synth_t *) (cs_cot), (amp))

#endif
