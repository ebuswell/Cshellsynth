/*
 * sampler.h
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
#ifndef CSHELLSYNTH_SAMPLER_H
#define CSHELLSYNTH_SAMPLER_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/jclient.h>
#include <stdbool.h>
#include <sndfile.h>

typedef struct cs_sampler_sndfile_struct {
    SNDFILE *sf;
    SF_INFO sf_info;
} cs_sampler_sf_t;

typedef struct cs_sampler_struct {
    jack_client_t *client;
    jack_port_t *ctl_port;
    jack_port_t *outL_port;
    jack_port_t *outR_port;
    atomic_ptr_t sf;
    atomic_t sf_sync;
    bool playing;
} cs_sampler_t;

int cs_sampler_destroy(cs_sampler_t *self);
int cs_sampler_init(cs_sampler_t *self, const char *client_name, jack_options_t flags, char *server_name);
int cs_sampler_load(cs_sampler_t *self, char *path);

#endif
