/*
 * triangle.h
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
#ifndef CSHELLSYNTH_TRIANGLE_H
#define CSHELLSYNTH_TRIANGLE_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/synth.h>

typedef struct cs_triangle_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    atomic_float_t freq;
    jack_port_t *out_port;
    atomic_float_t amp;
    atomic_float_t offset;
    double t;
} cs_triangle_t;

#define cs_triangle_destroy(cs_triangle) cs_synth_destroy((cs_synth_t *) (cs_triangle))
int cs_triangle_init(cs_triangle_t *self, const char *client_name, jack_options_t flags, char *server_name);
#define cs_triangle_set_freq(cs_triangle, freq) cs_synth_set_freq((cs_synth_t *) (cs_triangle), (freq))
#define cs_triangle_set_offset(cs_triangle, offset) cs_synth_set_offset((cs_synth_t *) (cs_triangle), (offset))
#define cs_triangle_set_amp(cs_triangle, amp) cs_synth_set_amp((cs_synth_t *) (cs_triangle), (amp))

#endif /* #ifndef CSHELLSYNTH_TRIANGLE_H */

