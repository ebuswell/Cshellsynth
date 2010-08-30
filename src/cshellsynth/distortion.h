/** @file distortion.h
 *
 * Distortion filter
 *
 * Ruby version: @c Filters::Distortion
 *
 * Limits input according to one of the following equations, where s is @c sharpness, g is
 * @c gain, and x is the original input.
 *
 * Lower values of @c sharpness make the sound less "warm" and vice versa.
 *
 * For <tt>Exponential</tt>:
 *
 * @verbatim
         -s(gx - 1)
    log(e           + 1)
1 - --------------------
            -s
       log(e   + 1)
@endverbatim
 *
 * With a symmetrical equation for a negative x.
 *
 * For <tt>Hyperbolic</tt>:
 *
 * @verbatim
      gx
--------------
     s     1/s
(|gx|  + 1)
@endverbatim
 *
 * For <tt>Arctangent</tt>:
 *
 * @verbatim
2atan(sgx)
----------
    π
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
#ifndef CSHELLSYNTH_DISTORTION_H
#define CSHELLSYNTH_DISTORTION_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/filter.h>

/**
 * Distortion filter
 *
 * Ruby version: @c Filters::Distortion
 *
 * See @ref cs_filter_t
 */
typedef struct cs_distort_struct {
    jack_client_t *client;
    jack_port_t *in_port;
    atomic_float_t in;
    jack_port_t *out_port;
    jack_port_t *gain_port;
    atomic_float_t gain;
    atomic_float_t sharpness;
    atomic_t kind;
} cs_distort_t;

/**
 * Destroy distortion filter
 *
 * See @ref cs_filter_destroy
 */
#define cs_distort_destroy(cs_distort) cs_filter_destroy((cs_filter_t *) (cs_distort))

/**
 * Initialize distortion filter
 *
 * See @ref cs_filter_init
 */
int cs_distort_init(cs_distort_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * Set which kind of distortion to use.
 *
 * Ruby version: @c kind=
 *
 * Ruby version of values is @c Filters::Distortion::Exponential, @c Filters::Distortion::Hyperbolic, and @c Filters::Distortion::Arctangent
 *
 * @param kind CS_EXP, CS_HYP, CS_ATAN
 *
 */
void cs_distort_set_kind(cs_distort_t *self, int kind);

/**
 * @ref cs_filter_set_in
 */
#define cs_distort_set_in(self, in) cs_filter_set_in(self, in)

/**
 * Set gain
 *
 * Ruby version: @c gain=
 *
 * @param gain the gain, from 0 to inf.  This is exactly equivalent to raising/lowering
 * the amplitude before the distortion stage.
 */
void cs_distort_set_gain(cs_distort_t *self, float gain);

/**
 * Set sharpness
 *
 * Ruby version: @c sharpness=
 *
 * @param sharpness how sharply the transfer curve bends towards 1.0.  Lower values sound
 * "warmer" and vice versa.
 */
void cs_distort_set_sharpness(cs_distort_t *self, float sharpness);

#define CS_EXP 1
#define CS_HYP 2
#define CS_ATAN 3
#define CS_TUBE 4

#endif
