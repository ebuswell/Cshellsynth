/** @file clock.h
 * Clock
 *
 * Ruby version: @c Clock
 *
 * The clock outputs a timestamp for each sample, in the range from [0,meter)
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
#ifndef CSHELLSYNTH_CLOCK_H
#define CSHELLSYNTH_CLOCK_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/jclient.h>

/**
 * Clock
 *
 * Ruby version: @c Clock
 *
 * The clock outputs a timestamp for each sample, in the range from [0,meter)
 *
 * See @ref jclient_t
 */
typedef struct cs_clock_struct {
    jack_client_t *client;
    jack_port_t *clock_port; /** Output clock port */
    jack_port_t *meter_port; /** Input meter port */
    atomic_float_t meter; /** Static version of meter */
    jack_port_t *rate_port; /** Input rate port */
    atomic_float_t rate; /** Static version of rate */
    double current; /** The current value */
} cs_clock_t;

/**
 * Destroy clock
 *
 * See @ref jclient_destroy
 */
#define cs_clock_destroy(cs_clock) jclient_destroy((jclient_t *) (cs_clock))

/**
 * Initialize clock
 *
 * See @ref jclient_init
 */
int cs_clock_init(cs_clock_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * Set meter
 *
 * Ruby version: @c meter=
 */
void cs_clock_set_meter(cs_clock_t *self, float meter);

/**
 * Set rate
 *
 * Ruby version: @c rate=
 *
 * @param rate the rate.  If <= 1, this is in beats per sample.  Otherwise, the value is
 * converted from beats per minute.
 *
 */
void cs_clock_set_rate(cs_clock_t *self, float rate);

#endif
