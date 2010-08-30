/** @file envelope_generator.h
 *
 * Envelope Generator
 *
 * Ruby version: @c EnvelopeGenerator
 *
 * Creates an envelope corresponding to a control signal.
 *
 * @verbatim
           _ attack_a
        _-- \
      --     \
    /          \________ sustain_a
   /                    \
 /                       \
/                          \
release_a                   release_a
|__________|___|       |___|
 attack_t   decay_t     release_t
@endverbatim
 *
 * The bad ascii art is trying to illustrate the default exponential version.
 *
 * If you want the addition of "hold" time, run the output through a distortion filter.
 * That's likely to give you a more releastic punch then adding in a hold parameter
 * anyway.
 *
 * Also, note that by setting the appropriate parameters you can invert the envelope or do
 * many other nonstandard things useful in controlling filters.  The predictable way that
 * a linear envelope interacts with Lin2Exp (@ref lin2exp.h) is important for certain effects.
 *
 * The attack and decay cycle are always performed.  A control signal to release during
 * this time will cause the release to happen immediately after the decay.  Conversely, a
 * control signal to attack starts the attack cycle immediately.
 *
 * As a last warning, I suspect the existence of bugs I have not yet been able to identify
 * when attack_a and release_a are not the default values.
 *
 * @todo parameterize a forced release.
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
#ifndef CSHELLSYNTH_ENVELOPE_GENERATOR_H
#define CSHELLSYNTH_ENVELOPE_GENERATOR_H 1

#include <jack/jack.h>
#include <stdbool.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/jclient.h>

/**
 * Envelope Generator State
 */
enum cs_envg_state {
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE,
    FINISHED
};

/**
 * Envelope Generator
 *
 * Ruby version: @c EnvelopeGenerator
 *
 * See @ref jclient_t
 */
typedef struct cs_envg_struct {
    jack_client_t *client;
    jack_port_t *ctl_port; /** Input control port */
    jack_port_t *out_port; /** Output */
    atomic_double_t attack_t; /** Attack time */
    atomic_float_t attack_a; /** Attack amplitude */
    atomic_double_t decay_t; /** Decay time */
    atomic_float_t sustain_a; /** Sustain amplitude */
    atomic_double_t release_t; /** Release time */
    atomic_float_t release_a; /** Release amplitude */
    atomic_t linear; /** Whether it's linear or exponential */
    bool upwards; /** Keeps track of which direction we're going */
    enum cs_envg_state state; /** Keeps track of the state */
    double last_a; /** The previous value */
    bool release; /** Whether we should release when apropos */
} cs_envg_t;

/**
 * Destroy envelope generator
 *
 * See @ref jclient_destroy
 */
#define cs_envg_destroy(cs_envg) jclient_destroy((jclient_t *) (cs_envg))

/**
 * Initialize envelope generator
 *
 * See @ref jclient_init
 */
int cs_envg_init(cs_envg_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * Set attack time
 *
 * Ruby version: @c attack_t=
 */
void cs_envg_set_attack_t(cs_envg_t *self, double attack_t);

/**
 * Set attack amplitude
 *
 * Ruby version: @c attack_a=
 */
void cs_envg_set_attack_a(cs_envg_t *self, float attack_a);

/**
 * Set decay time
 *
 * Ruby version: @c decay_t=
 */
void cs_envg_set_decay_t(cs_envg_t *self, double decay_t);

/**
 * Set sustain amplitude
 *
 * Ruby version: @c sustain_a=
 */
void cs_envg_set_sustain_a(cs_envg_t *self, float sustain_a);

/**
 * Set release time
 *
 * Ruby version: @c release_t=
 */
void cs_envg_set_release_t(cs_envg_t *self, double release_t);

/**
 * Set release amplitude
 *
 * Ruby version: @c release_a=
 */
void cs_envg_set_release_a(cs_envg_t *self, float release_a);

/**
 * Set whether the envelope is linear or exponential
 *
 * Ruby version: @c linear=
 *
 * @param linear 0 for exponential, anything else for linear
 */
void cs_envg_set_linear(cs_envg_t *self, int linear);

#endif
