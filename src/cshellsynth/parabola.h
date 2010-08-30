/** @file parabola.h
 *
 * Parabolic Wave Synth
 *
 * Ruby version: @c Synths::Parabola
 *
 * Makes a wave of the equation:
 *
 * @verbatim
 2                         2
π ( (ft - floor(ft)) - 0.5)  - 1/12 )
@endverbatim
 *
 * Which is equivalent to:
 *
 * @verbatim
inf             2
 Σ sin(n*wt) / n
n=1
@endverbatim
 *
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
#ifndef CSHELLSYNTH_PARABOLA_H
#define CSHELLSYNTH_PARABOLA_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/synth.h>

/**
 * Parabolic Wave Synth
 *
 * Ruby version: @c Synths::Parabola
 *
 * See @ref cs_synth_t
 */
typedef struct cs_parab_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    atomic_float_t freq;
    jack_port_t *out_port;
    atomic_float_t amp;
    atomic_float_t offset;
    double t; /** Time offset, as a fraction of wavelength */
} cs_parab_t;

/**
 * Destroy parabola synth
 *
 * See @ref cs_synth_destroy
 */
#define cs_parab_destroy(cs_parab) cs_synth_destroy((cs_synth_t *) (cs_parab))

/**
 * Initialize parabola synth
 *
 * See @ref cs_synth_destroy
 */
int cs_parab_init(cs_parab_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * @ref cs_synth_set_freq
 */
#define cs_parab_set_freq(cs_parab, freq) cs_synth_set_freq((cs_synth_t *) (cs_parab), (freq))

/**
 * @ref cs_synth_set_offset
 */
#define cs_parab_set_offset(cs_parab, offset) cs_synth_set_offset((cs_synth_t *) (cs_parab), (offset))

/**
 * @ref cs_synth_set_amp
 */
#define cs_parab_set_amp(cs_parab, amp) cs_synth_set_amp((cs_synth_t *) (cs_parab), (amp))

#endif
