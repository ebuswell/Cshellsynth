/** @file lin2exp.h
 *
 * Linear to Exponential Filter
 *
 * Ruby version: @c Filters::Lin2Exp
 *
 * Translates a linear change into an exponential change according to the equation:
 *
 * @verbatim
 *
 *   x
 * z2
 *
 * @endverbatim
 *
 * where x is the original input, and z is @c zero, the value when x is 0
 *
 * If z is a frequency, x is the number of octaves to shift that frequency.
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
#ifndef CSHELLSYNTH_LIN2EXP_H
#define CSHELLSYNTH_LIN2EXP_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/filter.h>

/**
 * Linear to Exponential Filter
 *
 * Ruby version: @c Filters::Lin2Exp
 *
 * See @ref cs_filter_t
 */
typedef struct cs_lin2exp_struct {
    jack_client_t *client;
    jack_port_t *in_port;
    atomic_float_t in;
    jack_port_t *out_port;
    jack_port_t *zero_port; /** The value when the input is 0 */
    atomic_float_t zero; /** Static version of zero */
} cs_lin2exp_t;

/**
 * Destroy distortion filter
 *
 * See @ref cs_filter_destroy
 */
#define cs_lin2exp_destroy(cs_lin2exp) cs_filter_destroy((cs_filter_t *) (cs_lin2exp))

/**
 * Initialize distortion filter
 *
 * See @ref cs_filter_init
 */
int cs_lin2exp_init(cs_lin2exp_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * @ref cs_filter_set_in
 */
#define cs_lin2exp_set_in(self, in) cs_filter_set_in((cs_filter_t *) (self), in)

/**
 * Set zero
 *
 * Ruby version: @c zero=
 *
 * @param zero the value when the input is 0.
 */
void cs_lin2exp_set_zero(cs_lin2exp_t *self, float zero);

#endif
