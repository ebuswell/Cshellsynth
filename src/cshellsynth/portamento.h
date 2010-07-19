/** @file portamento.h
 *
 * Portamento Filter
 *
 * Ruby version: @c Filters::Portamento
 *
 * Causes instantanous changes to instead linearly progress from the old to new value over
 * a time lag.
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
#ifndef CSHELLSYNTH_PORTAMENTO_H
#define CSHELLSYNTH_PORTAMENTO_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/filter.h>

/**
 * Portamento Filter
 *
 * Ruby version: @c Filters::Portamento
 *
 * See @ref cs_filter_t
 */
typedef struct cs_porta_struct {
    jack_client_t *client;
    jack_port_t *in_port;
    atomic_float_t in;
    jack_port_t *out_port;
    jack_port_t *lag_port; /** The lag, in samples */
    atomic_float_t lag; /** Static version of lag */
    double start; /** The value being progressed from */
    double target; /** The value being progressed to */
    double last; /** The previous value */
} cs_porta_t;

/**
 * Destroy portamento filter
 *
 * See @ref cs_filter_destroy
 */
#define cs_porta_destroy(cs_porta) cs_filter_destroy((cs_filter_t *) (cs_porta))

/**
 * Initialize portamento filter
 *
 * See @ref cs_filter_init
 */
int cs_porta_init(cs_porta_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * @ref cs_filter_set_in
 */
#define cs_porta_set_in(self, in) cs_filter_set_in(self, in)

/**
 * Set lag
 *
 * Ruby version: @c lag=
 *
 * @param lag the number of seconds it takes to go from the old value to the new.
 */
void cs_porta_set_lag(cs_porta_t *self, float lag);

#endif
