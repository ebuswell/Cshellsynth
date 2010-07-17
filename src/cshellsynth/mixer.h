/*
 * mixer.h
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
#ifndef CSHELLSYNTH_MIXER_H
#define CSHELLSYNTH_MIXER_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/jclient.h>

typedef struct cs_mix_struct {
    jack_client_t *client;
    jack_port_t *in1_port;
    atomic_float_t in1;
    jack_port_t *in2_port;
    atomic_float_t in2;
    jack_port_t *out_port;
    atomic_float_t in1_amp;
    atomic_float_t in2_amp;
} cs_mix_t;

#define cs_mix_destroy(cs_mix) jclient_destroy((jclient_t *) (cs_mix))
int cs_mix_init(cs_mix_t *self, const char *client_name, jack_options_t flags, char *server_name);
int cs_mix_subclass_init(cs_mix_t *self, const char *client_name, jack_options_t flags, char *server_name);
void cs_mix_set_in1(cs_mix_t *self, float in1);
void cs_mix_set_in2(cs_mix_t *self, float in2);
void cs_mix_set_in1_amp(cs_mix_t *self, float in1_amp);
void cs_mix_set_in2_amp(cs_mix_t *self, float in2_amp);

#endif
