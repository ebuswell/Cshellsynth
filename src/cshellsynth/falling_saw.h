/** @file falling_saw.h
 *
 * Falling Sawtooth Wave Synth
 *
 * Ruby version: @c Synths::FallingSaw
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
#ifndef CSHELLSYNTH_FALLING_SAW_H
#define CSHELLSYNTH_FALLING_SAW_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/synth.h>

/**
 * Falling Sawtooth Wave Synth
 *
 * Ruby version: @c Synths::FallingSaw
 *
 * See @ref cs_synth_t
 */
typedef struct cs_fsaw_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    atomic_float_t freq;
    jack_port_t *out_port;
    atomic_float_t amp;
    atomic_float_t offset;
    double t; /** Time offset, as a fraction of wavelength */
} cs_fsaw_t;

/**
 * Destroy falling saw synth
 *
 * See @ref cs_synth_destroy
 */
#define cs_fsaw_destroy(cs_fsaw) cs_synth_destroy((cs_synth_t *) (cs_fsaw))

/**
 * Initialize falling saw synth
 *
 * See @ref cs_synth_destroy
 */
int cs_fsaw_init(cs_fsaw_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * @ref cs_synth_set_freq
 */
#define cs_fsaw_set_freq(cs_fsaw, freq) cs_synth_set_freq((cs_synth_t *) (cs_fsaw), (freq))

/**
 * @ref cs_synth_set_offset
 */
#define cs_fsaw_set_offset(cs_fsaw, offset) cs_synth_set_offset((cs_synth_t *) (cs_fsaw), (offset))

/**
 * @ref cs_synth_set_amp
 */
#define cs_fsaw_set_amp(cs_fsaw, amp) cs_synth_set_amp((cs_synth_t *) (cs_fsaw), (amp))

#endif
