/*
 * jclient.h
 * 
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

typedef struct jclient_struct {
    jack_client_t *client;
} jclient_t;

int jclient_destroy(jclient_t *self);
int jclient_init(jclient_t *self, const char *client_name, jack_options_t flags, char *server_name);

#endif
