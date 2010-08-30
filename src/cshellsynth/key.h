/** @file key.h
 *
 * Key
 *
 * Ruby version: @c Key
 *
 * Key translates notes into frequencies, scaled by the sample frequency.
 *
 * It does so according to an array which corresponds to the scale:
 *
 * @verbatim
             n/l
rt[n % l] * 2
@endverbatim
 *
 * where r is the root, t is the tuning array, n is the note, and l is the length of the
 * array.  Note that n / l is truncated integer arithmetic, such that n / l is the integer
 * amount, n % l the remainder.
 *
 * Additionally, notes may be fractional.  Fractional notes correspond to the equation:
 *
 * @verbatim
      f
p(n/p)
@endverbatim
 *
 * where p is the previous note, n is the next note, and f is the fractional portion.
 *
 * Note that in the case of equal temperament, this all just reduces to:
 *
 * @verbatim
   n
r*2
@endverbatim
 *
 * Key comes with a number of pre-defined scales.  Minor, Major, Equal-Tempered, and
 * Pythagorean.  If you wish to define your own, pass an array of fractional values
 * between 1 and 2 that correspond to the notes in your scale.  If you have a favorite
 * somewhat conventional scale that you think should be predefined, file a bug or send an
 * email with the fractional coefficients of each note, and I'll probably add it.
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
#ifndef CSHELLSYNTH_KEY_H
#define CSHELLSYNTH_KEY_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/jclient.h>

/**
 * Tuning array
 */
typedef struct cs_key_tuning_struct {
    size_t tuning_length; /** The length of the array */
    const double *tuning; /** The array */
} cs_key_tuning_t;

/**
 * Key
 *
 * Ruby version: @c Key
 *
 * See @ref jclient_t
 */
typedef struct cs_key_struct {
    jack_client_t *client;
    jack_port_t *note_port; /** Input note */
    atomic_float_t note; /** Static version of note */
    jack_port_t *root_port; /** Root frequency, as a fraction of sample rate */
    atomic_float_t root; /** Static version of root */
    jack_port_t *freq_port; /** Output frequency, as a fraction of sample rate */
    atomic_ptr_t tuning; /** The tuning, of type cs_key_tuning_t */
    atomic_t tuning_sync; /** For synchronizing while switching tunings */
} cs_key_t;

/**
 * Destroy key
 *
 * See @ref jclient_destroy
 */
int cs_key_destroy(cs_key_t *cs_key);

/**
 * Initialize key
 *
 * @ref jclient_init
 */
int cs_key_init(cs_key_t *self, const char *client_name, jack_options_t flags, char *server_name);

/**
 * Set root
 *
 * Ruby version: @c root=
 *
 * @param root the frequency when note is zero.  If <= 1, as a fraction of sample rate;
 * otherwise the conventional frequency in Hz.
 *
 */
void cs_key_set_root(cs_key_t *self, float root);

/**
 * Set tuning
 *
 * Ruby version: @c tuning=
 *
 * @param tuning the tuning array.  A copy is made for internal use and the variable
 * passed is not referenced after return, unless using one of the predefined values.
 *
 * @param tuning_length the length of the tuning array.  It is possibly useful to pass
 * less than the actual length, to drop the additional notes off the end of the scale.
 */
int cs_key_set_tuning(cs_key_t *self, const double *tuning, size_t tuning_length);

/**
 * Transform a note into a frequency according to the semantics of this particular key.
 *
 * Ruby version: @c note2freq
 */
float cs_key_note2freq(cs_key_t *self, float note);

/**
 * Set note
 */
void cs_key_set_note(cs_key_t *self, float note);

/**
 * A
 *
 * Ruby version: @c Key::A
 */
#define CS_A 220.0

/**
 * A#
 *
 * Ruby version: @c Key::A_Sharp
 */
#define CS_A_SHARP 233.081880759045

/**
 * Bb
 *
 * Ruby version: @c Key::B_Flat
 */
#define CS_B_FLAT 233.081880759045

/**
 * B
 *
 * Ruby version: @c Key::B
 */
#define CS_B 246.941650628062

/**
 * Cb
 *
 * Ruby version: @c Key::C_Flat
 */
#define CS_C_FLAT 246.941650628062

/**
 * C
 *
 * Ruby version: @c Key::C
 */
#define CS_C 261.625565300599

/**
 * B#
 *
 * Ruby version: @c Key::B_Sharp
 */
#define CS_B_SHARP 261.625565300599

/**
 * C#
 *
 * Ruby version: @c Key::C_Sharp
 */
#define CS_C_SHARP 277.182630976872

/**
 * Db
 *
 * Ruby version: @c Key::D_Flat
 */
#define CS_D_FLAT 277.182630976872

/**
 * D
 *
 * Ruby version: @c Key::D
 */
#define CS_D 293.664767917408

/**
 * D#
 *
 * Ruby version: @c Key::D_Sharp
 */
#define CS_D_SHARP 311.126983722081

/**
 * Eb
 *
 * Ruby version: @c Key::E_Flat
 */
#define CS_E_FLAT 311.126983722081

/**
 * E
 *
 * Ruby version: @c Key::E
 */
#define CS_E 329.62755691287

/**
 * Fb
 *
 * Ruby version: @c Key::F_Flat
 */
#define CS_F_FLAT 329.62755691287

/**
 * F
 *
 * Ruby version: @c Key::F
 */
#define CS_F 349.228231433004

/**
 * E#
 *
 * Ruby version: @c Key::E_Sharp
 */
#define CS_E_SHARP 349.228231433004

/**
 * F#
 *
 * Ruby version: @c Key::F_Sharp
 */
#define CS_F_SHARP 369.994422711634

/**
 * Gb
 *
 * Ruby version: @c Key::G_Flat
 */
#define CS_G_FLAT 369.994422711634

/**
 * G
 *
 * Ruby version: @c Key::G
 */
#define CS_G 391.995435981749

/**
 * G#
 *
 * Ruby version: @c Key::G_Sharp
 */
#define CS_G_SHARP 415.304697579945

/**
 * Ab
 *
 * Ruby version: @c Key::A_Flat
 */
#define CS_A_FLAT 415.304697579945

/**
 * Major tuning
 *
 * Ruby version: @c Key::Major
 */
extern const double CS_MAJOR_TUNING[];
/**
 * Major tuning length
 */
#define CS_MAJOR_TUNING_LENGTH 7

/**
 * Minor tuning
 *
 * Ruby version: @c Key::Minor
 */
extern const double CS_MINOR_TUNING[];
/**
 * Minor tuning length
 */
#define CS_MINOR_TUNING_LENGTH 7

/**
 * Pythagorean tuning
 *
 * Ruby version: @c Key::Pythagorean
 */
extern const double CS_PYTHAGOREAN_TUNING[];
/**
 * Pythagorean tuning length
 */
#define CS_PYTHAGOREAN_TUNING_LENGTH 12

/**
 * Equal temperament
 *
 * Ruby version: @c Key::Equal
 */
#define CS_EQUAL_TUNING ((double *) -1)
/**
 * Equal temperament length
 */
#define CS_EQUAL_TUNING_LENGTH 12

#endif
