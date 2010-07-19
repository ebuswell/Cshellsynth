/** @file edho.h
 *
 * Exponentially Decreasing Harmonics Oscillator Synth
 *
 * Ruby version: @c Synths::Edho
 *
 * This produces a wave corresponding to the equation
 *
 * @verbatim
 *
 * inf  n
 *  E  b  * sin((n + 1)wt)
 * n=0
 *
 * @endverbatim
 *
 * Where b is "brightness", valued from 0 (a sine wave, @ref sine.h) to 1 (a
 * cotangent/2 wave, containing equal amounts of all frequencies, @ref cot.h).
 *
 * If the @c scale parameter is set, the amplitude of the wave will be decreased such
 * that the peak is always 1.0.
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
#ifndef CSHELLSYNTH_EDHO_H
#define CSHELLSYNTH_EDHO_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/synth.h>

/**
 * Exponentially Decreasing Harmonics Oscillator Synth
 *
 * Ruby version: @c Synths::Edho
 *
 * See @ref cs_synth_t
 */
typedef struct cs_edho_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    atomic_float_t freq;
    jack_port_t *out_port;
    atomic_float_t amp;
    atomic_float_t offset;
    double t; /** Time offset, as a fraction of wavelength */
    jack_port_t *bright_port; /** Bright, from 0-1, see discussion at @ref edho.h */
    atomic_float_t bright; /** Static version of bright_port */
    atomic_t scale; /** Whether or not to scale the maximum amplitude to 1 */
} cs_edho_t;

/**
 * Destroy edho synth
 *
 * See @ref cs_synth_destroy
 */
#define cs_edho_destroy(cs_edho) cs_synth_destroy((cs_synth_t *) (cs_edho))

/**
 * Initialize edho synth
 *
 * See @ref cs_synth_destroy
 */
int cs_edho_init(cs_edho_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * @ref cs_synth_set_freq
 */
#define cs_edho_set_freq(cs_edho, freq) cs_synth_set_freq((cs_synth_t *) (cs_edho), (freq))

/**
 * @ref cs_synth_set_offset
 */
#define cs_edho_set_offset(cs_edho, offset) cs_synth_set_offset((cs_synth_t *) (cs_edho), (offset))

/**
 * @ref cs_synth_set_amp
 */
#define cs_edho_set_amp(cs_edho, amp) cs_synth_set_amp((cs_synth_t *) (cs_edho), (amp))

/**
 * Set bright, from 0-1, see discussion at @ref edho.h
 *
 * Ruby version: @c bright=
 *
 * @param bright the brightness.  Default is 0.5.
 */
void cs_edho_set_bright(cs_edho_t *self, float bright);

/**
 * Set whether to scale to 1 or not
 *
 * Ruby version: @c scale=
 *
 * @param scale scale. 0 for not scaling, nonzero for scaling
 */
void cs_edho_set_scale(cs_edho_t *self, int scale);

#endif
