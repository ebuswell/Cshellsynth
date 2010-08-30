/** @file highpass.h
 *
 * Highpass filter
 *
 * Ruby version: @c Filters::Highpass
 *
 * @verbatim
H(s) = s^2 / (s^2 + s/Q + 1)
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
#ifndef CSHELLSYNTH_HIGHPASS_H
#define CSHELLSYNTH_HIGHPASS_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/filter.h>
#include <cshellsynth/lowpass.h>

/**
 * Highpass filter
 *
 * Ruby version: @c Filters::Highpass
 *
 * See @ref cs_lowpass_t
 */
typedef cs_lowpass_t cs_highpass_t;

/**
 * Destroy highpass filter
 *
 * See @ref cs_filter_destroy
 */
#define cs_highpass_destroy(cs_highpass) cs_filter_destroy((cs_filter_t *) (cs_highpass))

/**
 * Initialize highpass filter
 *
 * See @ref cs_filter_init
 */
int cs_highpass_init(cs_highpass_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * @ref cs_filter_set_in
 */
#define cs_highpass_set_in(self, in) cs_filter_set_in(self, in)

/**
 * @ref cs_lowpass_set_freq
 */
#define cs_highpass_set_freq(self, freq) cs_lowpass_set_freq(self, freq)

/**
 * @ref cs_lowpass_set_Q
 */
#define cs_highpass_set_Q(self, Q) cs_lowpass_set_Q(self, Q)

/**
 * @ref cs_lowpass_set_atten
 */
#define cs_highpass_set_atten(self, atten) cs_lowpass_set_atten(self, atten)

#endif
