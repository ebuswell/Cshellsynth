/*
 * instrument.h
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
#ifndef CSHELLSYNTH_INSTRUMENT_H
#define CSHELLSYNTH_INSTRUMENT_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/controller.h>

typedef struct cs_inst_struct {
    jack_client_t *client;
    jack_port_t *ctl_port;
    jack_port_t *out_port;
    atomic_float_t value;
    atomic_float_t ctl;
} cs_inst_t;

#define cs_inst_destroy(cs_inst) cs_ctlr_destroy((cs_ctlr_t *) (cs_inst))
int cs_inst_init(cs_inst_t *self, const char *client_name, jack_options_t flags, char *server_name);
void cs_inst_play(cs_inst_t *self, float value);
void cs_inst_stop(cs_inst_t *self);

#endif
