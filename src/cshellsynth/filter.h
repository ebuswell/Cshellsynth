/** @file filter.h
 *
 * Generic Filter
 *
 * Ruby version: @c Filters::Filter
 *
 * In Cshellsynth, a filter is just something which performs a transformation on the
 * input.  It is not necessary for it to involve a frequency parameter, but it often
 * does.
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
#ifndef CSHELLSYNTH_FILTER_H
#define CSHELLSYNTH_FILTER_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/jclient.h>

/**
 * Generic Filter
 *
 * Ruby version: @c Filters::Filter
 *
 * See @ref jclient_t
 */
typedef struct cs_filter_struct {
    jack_client_t *client;
    jack_port_t *in_port; /** Input */
    atomic_float_t in; /** Static version of input */
    jack_port_t *out_port; /** Output */
} cs_filter_t;

/**
 * Destroy Filter
 *
 * See @ref jclient_destroy
 */
#define cs_filter_destroy(cs_filter) jclient_destroy((jclient_t *) (cs_filter))

/**
 * Initialize Filter
 *
 * See @ref jclient_init
 */
int cs_filter_init(cs_filter_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * Set static input
 */
void cs_filter_set_in(cs_filter_t *self, float in);

#endif
