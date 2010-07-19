/** @file jclient.h
 *
 * Thin wrapper around jack_client_t
 *
 * Ruby version: @c JackClient
 *
 * The Ruby version includes many useful functions.  I don't feel like documenting them
 * all now, but chances are good that if you want a particular non-realtime jack function,
 * JackClient implements that same function with a very similar name and calling sequence.
 *
 * There are a number of conventions that all Ruby subclasses follow.  The name of a port
 * can be referenced in most places by either a string or a reference to the port object.
 * Setting two ports equal connects them.  When a class takes its input either from a port
 * or from a static variable, these are named the same, and only distinguished by whether
 * the argument is numeric or not.
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
#ifndef CSHELLSYNTH_JCLIENT_H
#define CSHELLSYNTH_JCLIENT_H 1

#include <jack/jack.h>

/**
 * Thin wrapper around jack_client_t
 *
 * Ruby version: @c JackClient
 */
typedef struct jclient_struct {
    jack_client_t *client;
} jclient_t;

/**
 * Destroy Jack Client
 *
 * @returns 0 on success, nonzero otherwise
 */
int jclient_destroy(jclient_t *self);

/**
 * Initialize Jack Client
 *
 * @p client_name, @p flags, and @p server_name are passed directly to Jack.
 *
 * @returns 0 on success, nonzero otherwise
 */
int jclient_init(jclient_t *self, const char *client_name, jack_options_t flags, char *server_name);

#endif
