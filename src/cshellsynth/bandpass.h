/** @file bandpass.h
 *
 * Bandpass filter
 *
 * Ruby version: @c Filters::Bandpass
 *
 * The discrete equivalent of an RLC circuit, like so:
 *
 * @verbatim
 *
 * o--@-@-@---| |-----o
 *                 |
 *                 \
 *                 /
 *                 \
 *                _|_
 *                 -
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
#ifndef CSHELLSYNTH_BANDPASS_H
#define CSHELLSYNTH_BANDPASS_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/filter.h>

/**
 * Bandpass filter
 *
 * Ruby version: @c Filters::Bandpass
 *
 * See @ref cs_filter_t
 */
typedef struct cs_bandpass_struct {
    jack_client_t *client;
    jack_port_t *in_port;
    atomic_float_t in;
    jack_port_t *out_port;
    jack_port_t *freq_port; /** The corner frequency */
    atomic_float_t freq; /** The center frequency */
    jack_port_t *Q_port; /** The filter's Q */
    atomic_float_t Q; /** Static version of Q */
    double Exy; /** Sum of input minus output */
    double Ey; /** Sum of output */
    double EEyy; /** Sum of @ref Ey plus output */
} cs_bandpass_t;

/**
 * Destroy bandpass filter
 *
 * See @ref cs_filter_destroy
 */
#define cs_bandpass_destroy(cs_bandpass) cs_filter_destroy((cs_filter_t *) (cs_bandpass))

/**
 * Initialize bandpass filter
 *
 * See @ref cs_filter_init
 */
int cs_bandpass_init(cs_bandpass_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * @ref cs_filter_set_in
 */
#define cs_bandpass_set_in(self, in) cs_filter_set_in(self, in)

/**
 * Set center frequency
 *
 * Ruby version: @c freq=
 */
void cs_bandpass_set_freq(cs_bandpass_t *self, float freq);

/**
 * Set filter Q
 *
 * Ruby version: @c Q=
 */
void cs_bandpass_set_Q(cs_bandpass_t *self, float Q);

#endif
