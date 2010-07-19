/** @file square.h
 *
 * Square Wave Synth
 *
 * Ruby version: @c Synths::Square
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
#ifndef CSHELLSYNTH_SQUARE_H
#define CSHELLSYNTH_SQUARE_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/synth.h>

/** @file */

/**
 * Square Wave Synth
 *
 * Ruby version: @c Synths::Square
 *
 * See @ref cs_synth_t
 */
typedef struct cs_square_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    atomic_float_t freq;
    jack_port_t *out_port;
    atomic_float_t amp;
    atomic_float_t offset;
    atomic_double_t duty_cycle; /** Fraction of time the wave spends at 1.0. */
    double t; /** Time offset, as a fraction of wavelength */
} cs_square_t;

/**
 * Destroy square synth
 *
 * See @ref cs_synth_destroy
 */
#define cs_square_destroy(cs_square) cs_synth_destroy((cs_synth_t *) (cs_square))

/**
 * Initialize square synth
 *
 * See @ref cs_synth_init
 */
int cs_square_init(cs_square_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * @ref cs_synth_set_freq
 */
#define cs_square_set_freq(cs_square, freq) cs_synth_set_freq((cs_synth_t *) (cs_square), (freq))

/**
 * @ref cs_synth_set_offset
 */
#define cs_square_set_offset(cs_square, offset) cs_synth_set_offset((cs_synth_t *) (cs_square), (offset))

/**
 * @ref cs_synth_set_amp
 */
#define cs_square_set_amp(cs_square, amp) cs_synth_set_amp((cs_synth_t *) (cs_square), (amp))

/**
 * Sets the fraction of time the wave spends at 1.0.
 *
 * Ruby version: @c duty_cycle=
 *
 * @param duty_cycle duty cycle.  0.5 by default.
 */
void cs_square_set_duty_cycle(cs_square_t *self, double duty_cycle);

#endif
