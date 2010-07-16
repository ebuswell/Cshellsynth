/*
 * envelope_generator.h
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
#ifndef CSHELLSYNTH_ENVELOPE_GENERATOR_H
#define CSHELLSYNTH_ENVELOPE_GENERATOR_H 1

#include <jack/jack.h>
#include <stdbool.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/jclient.h>

enum cs_envg_state {
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE,
    FINISHED
};

typedef struct cs_envg_struct {
    jack_client_t *client;
    jack_port_t *ctl_port;
    jack_port_t *out_port;
    atomic_double_t attack_t;
    atomic_double_t decay_t;
    atomic_float_t sustain_a;
    atomic_double_t release_t;
    atomic_t linear;
    enum cs_envg_state state;
    double last_a;
    bool release;
} cs_envg_t;

#define cs_envg_destroy(cs_envg) jclient_destroy((jclient_t *) (cs_envg))
int cs_envg_init(cs_envg_t *self, const char *client_name, jack_options_t flags, char *server_name);
void cs_envg_set_attack_t(cs_envg_t *self, double attack_t);
void cs_envg_set_decay_t(cs_envg_t *self, double decay_t);
void cs_envg_set_sustain_a(cs_envg_t *self, float sustain_a);
void cs_envg_set_release_t(cs_envg_t *self, double release_t);
void cs_envg_set_linear(cs_envg_t *self, int linear);

#endif
