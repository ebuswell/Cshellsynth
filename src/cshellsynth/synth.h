/*
 * synth.h
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
#ifndef CSHELLSYNTH_SYNTH_H
#define CSHELLSYNTH_SYNTH_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/jclient.h>

typedef struct cs_synth_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    atomic_float_t freq;
    jack_port_t *out_port;
    atomic_float_t amp;
    atomic_float_t offset;
} cs_synth_t;

#define cs_synth_destroy(cs_synth) jclient_destroy((jclient_t *) (cs_synth))
int cs_synth_init(cs_synth_t *self, const char *client_name, jack_options_t flags, char *server_name);
void cs_synth_set_freq(cs_synth_t *self, float freq);
void cs_synth_set_offset(cs_synth_t *self, float offset);
void cs_synth_set_amp(cs_synth_t *self, float amp);

#endif
