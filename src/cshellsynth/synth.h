/** @file synth.h
 *
 * Structure for generic synth functions
 *
 * Ruby version: @c Synths::Synth
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
#ifndef CSHELLSYNTH_SYNTH_H
#define CSHELLSYNTH_SYNTH_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/jclient.h>

/**
 * Structure for generic synth functions
 *
 * Ruby version: @c Synths::Synth
 *
 * See @ref jclient_t
 */
typedef struct cs_synth_struct {
    jack_client_t *client;
    jack_port_t *freq_port; /** Frequency divided by sample rate */
    atomic_float_t freq; /** Static version of freq_port */
    jack_port_t *out_port; /** Output */
    atomic_float_t amp; /** Amplitude */
    atomic_float_t offset; /** Offset of the wave from zero */
} cs_synth_t;

/**
 * Destroy synth
 *
 * See @ref jclient_destroy
 */
#define cs_synth_destroy(cs_synth) jclient_destroy((jclient_t *) (cs_synth))

/**
 * Initialize synth
 *
 * See @ref jclient_init.
 */
int cs_synth_init(cs_synth_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * Set static synth frequency
 *
 * Ruby version @c freq=
 *
 * @param freq either a fraction of sampling frequency (<= 1.0) or an actual frequency (> 1.0)
 */
void cs_synth_set_freq(cs_synth_t *self, float freq);

/**
 * Set offset of the wave
 *
 * Ruby version @c offset=
 *
 * @param offset offset.  Default is 0.
 */
void cs_synth_set_offset(cs_synth_t *self, float offset);

/**
 * Set amplitude of the wave
 *
 * Ruby version @c amp=
 *
 * @param amp amplitude.  Default is 1.
 */
void cs_synth_set_amp(cs_synth_t *self, float amp);

#endif
