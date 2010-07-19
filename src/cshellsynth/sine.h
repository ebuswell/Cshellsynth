/** @file sine.h
 *
 * Sine Wave Synth
 *
 * Ruby version: @c Synths::Sine
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
#ifndef CSHELLSYNTH_SINE_H
#define CSHELLSYNTH_SINE_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/synth.h>

/** @file */

/**
 * Sine Wave Synth
 *
 * Ruby version: @c Synths::Sine
 *
 * See @ref cs_synth_t
 */
typedef struct cs_sine_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    atomic_float_t freq;
    jack_port_t *out_port;
    atomic_float_t amp;
    atomic_float_t offset;
    double t; /** Time offset, as a fraction of wavelength */
} cs_sine_t;

/**
 * Destroy sine synth
 *
 * See @ref cs_synth_destroy
 */
#define cs_sine_destroy(cs_sine) cs_synth_destroy((cs_synth_t *) (cs_sine))

/**
 * Initialize sine synth
 *
 * See @ref cs_synth_destroy
 */
int cs_sine_init(cs_sine_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * @ref cs_synth_set_freq
 */
#define cs_sine_set_freq(cs_sine, freq) cs_synth_set_freq((cs_synth_t *) (cs_sine), (freq))

/**
 * @ref cs_synth_set_offset
 */
#define cs_sine_set_offset(cs_sine, offset) cs_synth_set_offset((cs_synth_t *) (cs_sine), (offset))

/**
 * @ref cs_synth_set_amp
 */
#define cs_sine_set_amp(cs_sine, amp) cs_synth_set_amp((cs_synth_t *) (cs_sine), (amp))

#endif
