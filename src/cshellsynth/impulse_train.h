/** @file impulse_train.h
 *
 * Impulse Train Synth
 *
 * Ruby version: @c Synths::ImpulseTrain
 *
 * This produces an impulse train, which corresponds to:
 *
 * @verbatim
inf
 Σ cos(n*wt)
n=1
@endverbatim
 *
 * If the @c scale parameter is set, the (bandlimited) amplitude of the wave will be
 * decreased such that the peak is always under 1.0.  This is probably not what you want
 * unless you are using a constant frequency value.
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
#ifndef CSHELLSYNTH_IMPULSE_TRAIN_H
#define CSHELLSYNTH_IMPULSE_TRAIN_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/synth.h>

/**
 * Impulse Train Synth
 *
 * Ruby version: @c Synths::ImpulseTrain
 *
 * See @ref cs_synth_t
 */
typedef struct cs_itrain_struct {
    jack_client_t *client;
    jack_port_t *freq_port;
    atomic_float_t freq;
    jack_port_t *out_port;
    atomic_float_t amp;
    atomic_float_t offset;
    double t; /** Time offset, as a fraction of wavelength */
    atomic_t scale; /** Whether or not to scale the maximum amplitude to 1 */
} cs_itrain_t;

/**
 * Destroy impulse train synth
 *
 * See @ref cs_synth_destroy
 */
#define cs_itrain_destroy(cs_itrain) cs_synth_destroy((cs_synth_t *) (cs_itrain))

/**
 * Initialize impulse train synth
 *
 * See @ref cs_synth_destroy
 */
int cs_itrain_init(cs_itrain_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * @ref cs_synth_set_freq
 */
#define cs_itrain_set_freq(cs_itrain, freq) cs_synth_set_freq((cs_synth_t *) (cs_itrain), (freq))

/**
 * @ref cs_synth_set_offset
 */
#define cs_itrain_set_offset(cs_itrain, offset) cs_synth_set_offset((cs_synth_t *) (cs_itrain), (offset))

/**
 * @ref cs_synth_set_amp
 */
#define cs_itrain_set_amp(cs_itrain, amp) cs_synth_set_amp((cs_synth_t *) (cs_itrain), (amp))

/**
 * Set whether to scale the bandlimited waveform to 1 or not.  This is probably not what
 * you want unless the frequency is constant.  For manual scaling, note that if a given
 * frequency doesn't clip, no frequency above that will clip.
 *
 * Ruby version: @c scale=
 *
 * @param scale scale. 0 for not scaling, nonzero for scaling
 */
void cs_itrain_set_scale(cs_itrain_t *self, int scale);

#endif
