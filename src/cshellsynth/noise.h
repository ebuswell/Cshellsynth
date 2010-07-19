/** @file noise.h
 *
 * Noise Generator
 *
 * Ruby version: @c Synths::Noise
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
#ifndef CSHELLNOISE_NOISE_H
#define CSHELLNOISE_NOISE_H 1

#include <jack/jack.h>
#include <cshellsynth/jclient.h>
#include <cshellsynth/atomic-types.h>

/**
 * Noise Generator
 *
 * Ruby version: @c Synths::Noise
 * 
 * See @ref cs_synth_t, although this is not a strict subclass since there's no @p freq input.
 */
typedef struct cs_noise_struct {
    jack_client_t *client;
    jack_port_t *out_port;
    atomic_float_t amp;
    atomic_float_t offset;
    atomic_t kind; /** pink, white, or red */
    float state[3]; /** state information for the pink noise generator */
} cs_noise_t;

/**
 * Destroy noise generator
 */
#define cs_noise_destroy(cs_noise) jclient_destroy((jclient_t *) (cs_noise))

/**
 * Initialize noise generator
 */
int cs_noise_init(cs_noise_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * Set which kind of noise to generate.
 *
 * Ruby version: @c kind=
 *
 * Ruby version of values is @c Synths::Noise::White, @c Synths::Noise::Pink, and @c Synths::Noise::Red
 *
 * @param kind CS_WHITE, CS_PINK, or CS_RED.  Red is currently unsupported.
 *
 */
void cs_noise_set_kind(cs_noise_t *self, int kind);

/**
 * @ref cs_synth_set_offset
 */
void cs_noise_set_offset(cs_noise_t *self, float offset);

/**
 * @ref cs_synth_set_amp
 */
void cs_noise_set_amp(cs_noise_t *self, float amp);

#define CS_WHITE 1
#define CS_PINK 2
#define CS_RED 3

#endif
