/** @file sampler.h
 *
 * Simple Sampler
 *
 * Ruby version: @c Sampler
 *
 * Loads some sound file and then plays it when @c ctl is triggered.  Since there is no
 * frequency parameter, obviously no resampling is done.
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

/**
 * Simple Sampler
 *
 * Ruby version: @c Sampler
 *
 * @ref jclient_t
 */
typedef struct cs_sampler_struct {
    jack_client_t *client;
    jack_port_t *ctl_port; /** The port for the control signal */
    jack_port_t *outL_port; /** Left output port */
    jack_port_t *outR_port; /** Right output port */
    atomic_ptr_t sf; /** The soundfile */
    atomic_t sf_sync; /** A lock to synchronize sample changes */
    bool playing; /** Whether the sampler is playing or not */
} cs_sampler_t;

/**
 * Destroy sampler
 *
 * @ref jclient_destroy
 */
int cs_sampler_destroy(cs_sampler_t *self);

/**
 * Initialize sampler
 *
 * @ref jclient_init
 */
int cs_sampler_init(cs_sampler_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * Load a file into the sampler
 *
 * @param path the path where the sample is stored.  This will load anything that
 * libsndfile can read---so <em>no proprietary formats</em>.
 */
int cs_sampler_load(cs_sampler_t *self, char *path);

#endif
