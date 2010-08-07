/** @file pan.h
 *
 * Pan
 *
 * Ruby version: @c Pan
 *
 * Takes a mono input and trasforms it to a stereo output panned accordingly.
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
#ifndef CSHELLSYNTH_PAN_H
#define CSHELLSYNTH_PAN_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/jclient.h>

/**
 * Pan
 *
 * Ruby version: @c Pan
 *
 * See @ref jclient_t
 */
typedef struct cs_pan_struct {
    jack_client_t *client;
    jack_port_t *in_port; /** Input */
    jack_port_t *outL_port; /** Left output */
    jack_port_t *outR_port; /** Right output */
    jack_port_t *pan_port; /** Pan */
    atomic_float_t pan; /** Static version of pan */
} cs_pan_t;

/**
 * Destroy pan
 *
 * See @ref jclient_destroy
 */
#define cs_pan_destroy(cs_pan) jclient_destroy((jclient_t *) (cs_pan))

/**
 * Initialize pan
 *
 * See @ref jclient_init
 */
int cs_pan_init(cs_pan_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * Set pan amount
 *
 * @param pan -1.0 to 1.0; negative is left, positive is right.
 */
void cs_pan_set_pan(cs_pan_t *self, float pan);

#endif
