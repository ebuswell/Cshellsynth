/** @file triangle.h
 *
 * Triangle Wave Synth
 *
 * Ruby version: @c Synths::Triangle
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
#ifndef CSHELLSYNTH_TRIANGLE_H
#define CSHELLSYNTH_TRIANGLE_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/synth.h>

/**
 * Triangle Wave Synth
 *
 * Ruby version: @c Synths::Triangle
 *
 * See @ref cs_synth_t
 */
typedef struct cs_triangle_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    atomic_float_t freq;
    jack_port_t *out_port;
    atomic_float_t amp;
    atomic_float_t offset;
    atomic_double_t slope; /** See @cs_triangle_set_slope */
    double t; /** Time offset, as a fraction of wavelength */
} cs_triangle_t;

/**
 * Destroy triangle synth
 *
 * See @ref cs_synth_destroy
 */
#define cs_triangle_destroy(cs_triangle) cs_synth_destroy((cs_synth_t *) (cs_triangle))

/**
 * Initialize triangle synth
 *
 * See @ref cs_synth_init
 */
int cs_triangle_init(cs_triangle_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * @ref cs_synth_set_freq
 */
#define cs_triangle_set_freq(cs_triangle, freq) cs_synth_set_freq((cs_synth_t *) (cs_triangle), (freq))

/**
 * @ref cs_synth_set_offset
 */
#define cs_triangle_set_offset(cs_triangle, offset) cs_synth_set_offset((cs_synth_t *) (cs_triangle), (offset))

/**
 * @ref cs_synth_set_amp
 */
#define cs_triangle_set_amp(cs_triangle, amp) cs_synth_set_amp((cs_synth_t *) (cs_triangle), (amp))

/**
 * Sets the relative rising and falling slope.  For a normal triangle, this is 0.5.  For a
 * rising or falling saw, this is 0 or 1.  Note, however, that since this is implemented
 * as a comb filter, at 0 or 1 you will get silence.
 *
 * Ruby version: @c slope=
 *
 * @param slope relative slope.  0.5 by default.
 */
void cs_triangle_set_slope(cs_triangle_t *self, double slope);

#endif /* #ifndef CSHELLSYNTH_TRIANGLE_H */

