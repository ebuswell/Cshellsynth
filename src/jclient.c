/*
 * jclient.c
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
#include <jack/jack.h>
#include <pthread.h>
#include "cshellsynth/jclient.h"

int jclient_init(jclient_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    jack_status_t status;
    if(server_name == NULL) {
	self->client = jack_client_open(client_name, flags, &status);
    } else {
	self->client = jack_client_open(client_name, flags, &status, server_name);
    }
    if(self->client == NULL) {
	return status;
    } else {
	return 0;
    }
}

int jclient_destroy(jclient_t *self) {
    return jack_client_close(self->client);
}
