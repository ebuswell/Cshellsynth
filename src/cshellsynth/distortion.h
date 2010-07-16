/*
 * distortion.h
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
#ifndef CSHELLSYNTH_DISTORTION_H
#define CSHELLSYNTH_DISTORTION_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/filter.h>

typedef struct cs_distort_struct {
    jack_client_t *client;
    jack_port_t *in_port;
    atomic_float_t in;
    jack_port_t *out_port;
    jack_port_t *gain_port;
    atomic_float_t gain;
    atomic_float_t sharpness;
} cs_distort_t;

#define cs_distort_destroy(cs_distort) cs_filter_destroy((cs_filter_t *) (cs_distort))
int cs_distort_init(cs_distort_t *self, const char *client_name, jack_options_t flags, char *server_name);
#define cs_distort_set_in(self, in) cs_filter_set_in(self, in)
void cs_distort_set_gain(cs_distort_t *self, float gain);
void cs_distort_set_sharpness(cs_distort_t *self, float sharpness);

#endif
