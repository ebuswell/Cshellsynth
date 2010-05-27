#ifndef KEY_H
#define KEY_H 1

#include <jack/jack.h>
#include <pthread.h>
#include "jclient.h"

typedef struct cs_key_struct {
    jack_client_t *client;
    pthread_mutex_t *lock;
    jack_port_t *note_port;
    jack_port_t *freq_port;
    size_t tuning_length;
    jack_default_audio_sample_t *tuning;
    jack_default_audio_sample_t root;
} cs_key_t;

#define cs_key_destroy(cs_key) jclient_locking_destroy((jclient_locking_t *) (cs_key))
int cs_key_init(cs_key_t *self, const char *client_name, jack_options_t flags, char *server_name);
int cs_key_set_root(cs_key_t *self, jack_default_audio_sample_t root);
int cs_key_set_tuning(cs_key_t *self, jack_default_audio_sample_t *tuning, size_t tuning_length);
jack_default_audio_sample_t cs_key_note_freq(cs_key_t *self, jack_default_audio_sample_t note);

#define CS_A 220.0
#define CS_A_SHARP 233.081880759045
#define CS_B_FLAT 233.081880759045
#define CS_B 246.941650628062
#define CS_C_FLAT 246.941650628062
#define CS_C 261.625565300599
#define CS_B_SHARP 261.625565300599
#define CS_C_SHARP 277.182630976872
#define CS_D_FLAT 277.182630976872
#define CS_D 293.664767917408
#define CS_D_SHARP 311.126983722081
#define CS_E_FLAT 311.126983722081
#define CS_E 329.62755691287
#define CS_F_FLAT 329.62755691287
#define CS_F 349.228231433004
#define CS_E_SHARP 349.228231433004
#define CS_F_SHARP 369.994422711634
#define CS_G_FLAT 369.994422711634
#define CS_G 391.995435981749
#define CS_G_SHARP 415.304697579945
#define CS_A_FLAT 415.304697579945

const jack_default_audio_sample_t *CS_MAJOR_TUNING;
#define CS_MAJOR_TUNING_SIZE 7

const jack_default_audio_sample_t *CS_MINOR_TUNING;
#define CS_MINOR_TUNING_SIZE 7

const jack_default_audio_type *CS_PYTHAGOREAN_TUNING;
#define CS_PYTHAGOREAN_TUNING_SIZE 12

#define CS_EQUAL_TUNING ((jack_default_audio_t *) -1)
#define CS_EQUAL_TUNING_SIZE 12

#endif
