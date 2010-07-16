/*
 * portamento.h
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
#ifndef CSHELLSYNTH_PORTAMENTO_H
#define CSHELLSYNTH_PORTAMENTO_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/filter.h>

typedef struct cs_porta_struct {
    jack_client_t *client;
    jack_port_t *in_port;
    atomic_float_t in;
    jack_port_t *out_port;
    jack_port_t *lag_port;
    atomic_float_t lag;
    double start;
    double target;
    double last;
} cs_porta_t;

#define cs_porta_destroy(cs_porta) cs_filter_destroy((cs_filter_t *) (cs_porta))
int cs_porta_init(cs_porta_t *self, const char *client_name, jack_options_t flags, char *server_name);
#define cs_porta_set_in(self, in) cs_filter_set_in(self, in)
void cs_porta_set_lag(cs_porta_t *self, float lag);

#endif
