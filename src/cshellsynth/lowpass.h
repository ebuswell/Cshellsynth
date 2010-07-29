/** @file lowpass.h
 *
 * Lowpass filter
 *
 * Ruby version: @c Filters::Lowpass
 *
 * The discrete equivalent of an RL circuit, like so:
 *
 * @verbatim
 *
 * o--@-@-@------o
 *            |
 *            \
 *            /
 *            \
 *           _|_
 *            -
 *
 * @endverbatim
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
#ifndef CSHELLSYNTH_LOWPASS_H
#define CSHELLSYNTH_LOWPASS_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/filter.h>

/**
 * Lowpass filter
 *
 * Ruby version: @c Filters::Lowpass
 *
 * See @ref cs_filter_t
 */
typedef struct cs_lowpass_struct {
    jack_client_t *client;
    jack_port_t *in_port;
    atomic_float_t in;
    jack_port_t *out_port;
    jack_port_t *freq_port; /** The corner frequency */
    atomic_float_t freq; /** Static version of frequency */
    double Exy; /** Sum of input minus output values */
} cs_lowpass_t;

/**
 * Destroy lowpass filter
 *
 * See @ref cs_filter_destroy
 */
#define cs_lowpass_destroy(cs_lowpass) cs_filter_destroy((cs_filter_t *) (cs_lowpass))

/**
 * Initialize lowpass filter
 *
 * See @ref cs_filter_init
 */
int cs_lowpass_init(cs_lowpass_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * @ref cs_filter_set_in
 */
#define cs_lowpass_set_in(self, in) cs_filter_set_in(self, in)

/**
 * Set corner frequency
 *
 * Ruby version: @c freq=
 */
void cs_lowpass_set_freq(cs_lowpass_t *self, float freq);

#endif
