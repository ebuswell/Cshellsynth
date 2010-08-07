/** @file mixer.h
 *
 * Mixer
 *
 * Ruby version: @c LLMixer.  There is also a derived Ruby @c Mixer class described below.
 *
 * This simple mixer class just adds its arguments together, scaled by their respective
 * amplitudes.  Additionally, it serves as the subclass of all things that mix two things
 * to get one thing.  Right now, that's just Modulator (@ref modulator.h).
 *
 * In Ruby, there is a @c Mixer class which functions identically except that instead of
 * @c in1* and @c in2*, there are <tt>in[0-x]</tt> and <tt>amp[0-x]</tt>.  These are created on
 * demand, but note that all intermediate values are filled in, so <tt>in[3]</tt> will ensure
 * that <tt>in[0]</tt>, <tt>in[1]</tt>, and <tt>in[2]</tt> exist.
 *
 * Additionally, there is a FullMixer class which also has <tt>pan[0-x]</tt>, with @c outL
 * and @c outR instead of @c out.
 *
 * By default, $mixer is created as a FullMixer, with outL and outR connected to the
 * default output.
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
#ifndef CSHELLSYNTH_MIXER_H
#define CSHELLSYNTH_MIXER_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/jclient.h>

/**
 * Mixer
 *
 * Ruby version: @c LLMixer.  There is also a derived Ruby @c Mixer class described in @ref mixer.h.
 *
 * See @ref jclient_t
 */
typedef struct cs_mix_struct {
    jack_client_t *client;
    jack_port_t *in1_port; /** The first input */
    atomic_float_t in1; /** Static version of in1 */
    jack_port_t *in2_port; /** The second input */
    atomic_float_t in2; /** Static version of in2 */
    jack_port_t *out_port; /** Ouput */
    atomic_float_t in1_amp; /** Amplitude of first input */
    atomic_float_t in2_amp; /** Amplitude of second input */
} cs_mix_t;

/**
 * Destroy mixer
 *
 * See @ref jclient_destroy
 */
#define cs_mix_destroy(cs_mix) jclient_destroy((jclient_t *) (cs_mix))

/**
 * Initialize mixer
 *
 * See @ref jclient_init
 */
int cs_mix_init(cs_mix_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * An factored out init function for subclasses
 */
int cs_mix_subclass_init(cs_mix_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * Sets in1
 *
 * Ruby version: @c in1=
 */
void cs_mix_set_in1(cs_mix_t *self, float in1);

/**
 * Sets in2
 *
 * Ruby version: @c in2=
 */
void cs_mix_set_in2(cs_mix_t *self, float in2);

/**
 * Sets in1 amplitude
 *
 * Ruby version: @c in1amp=
 */
void cs_mix_set_in1_amp(cs_mix_t *self, float in1_amp);

/**
 * Sets in2 amplitude
 *
 * Ruby version: @c in2amp=
 */
void cs_mix_set_in2_amp(cs_mix_t *self, float in2_amp);

#endif
