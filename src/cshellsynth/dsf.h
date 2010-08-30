/** @file dsf.h
 *
 * Discrete Summation Formula Synth
 *
 * Ruby version: @c Synths::Dsf
 *
 * This produces a wave corresponding to the equation
 *
 * @verbatim
inf  n-1
 Σ  b    * sin(n*wt)
n=1
@endverbatim
 *
 * Where b is "brightness", valued from 0 (a sine wave, @ref sine.h) to 1 (a
 * cotangent/2 wave, containing equal amounts of all frequencies, @ref cot.h).
 *
 * If the @c scale parameter is set, the amplitude of the wave will be decreased such
 * that the peak is always under 1.0.
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
#ifndef CSHELLSYNTH_DSF_H
#define CSHELLSYNTH_DSF_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/synth.h>

/**
 * Discrete Summation Formula Synth
 *
 * Ruby version: @c Synths::Dsf
 *
 * See @ref cs_synth_t
 */
typedef struct cs_dsf_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    atomic_float_t freq;
    jack_port_t *out_port;
    atomic_float_t amp;
    atomic_float_t offset;
    double t; /** Time offset, as a fraction of wavelength */
    jack_port_t *bright_port; /** Bright, from 0-1, see discussion at @ref dsf.h */
    atomic_float_t bright; /** Static version of bright_port */
    atomic_t scale; /** Whether or not to scale the maximum amplitude to 1 */
} cs_dsf_t;

/**
 * Destroy dsf synth
 *
 * See @ref cs_synth_destroy
 */
#define cs_dsf_destroy(cs_dsf) cs_synth_destroy((cs_synth_t *) (cs_dsf))

/**
 * Initialize dsf synth
 *
 * See @ref cs_synth_destroy
 */
int cs_dsf_init(cs_dsf_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * @ref cs_synth_set_freq
 */
#define cs_dsf_set_freq(cs_dsf, freq) cs_synth_set_freq((cs_synth_t *) (cs_dsf), (freq))

/**
 * @ref cs_synth_set_offset
 */
#define cs_dsf_set_offset(cs_dsf, offset) cs_synth_set_offset((cs_synth_t *) (cs_dsf), (offset))

/**
 * @ref cs_synth_set_amp
 */
#define cs_dsf_set_amp(cs_dsf, amp) cs_synth_set_amp((cs_synth_t *) (cs_dsf), (amp))

/**
 * Set bright, from 0-1, see discussion at @ref dsf.h
 *
 * Ruby version: @c bright=
 *
 * @param bright the brightness.  Default is 0.5.
 */
void cs_dsf_set_bright(cs_dsf_t *self, float bright);

/**
 * Set whether to scale to 1 or not
 *
 * Ruby version: @c scale=
 *
 * @param scale scale. 0 for not scaling, nonzero for scaling
 */
void cs_dsf_set_scale(cs_dsf_t *self, int scale);

#endif
