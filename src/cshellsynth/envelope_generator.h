#ifndef CSHELLSYNTH_ENVELOPE_GENERATOR_H
#define CSHELLSYNTH_ENVELOPE_GENERATOR_H 1

#include <jack/jack.h>
#include <pthread.h>
#include <cshellsynth/jclient.h>

enum cs_envg_state {
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE,
    FINISHED
};

typedef struct cs_envg_struct {
    jack_client_t *client;
    pthread_mutex_t lock;
    jack_port_t *ctl_port;
    jack_port_t *out_port;
    double attack_t;
    double decay_t;
    jack_default_audio_sample_t sustain_a;
    double release_t;
    double offset;
    enum cs_envg_state state;
} cs_envg_t;

#define cs_envg_destroy(cs_envg) jclient_locking_destroy((jclient_locking_t *) (cs_envg))
int cs_envg_init(cs_envg_t *self, const char *client_name, jack_options_t flags, char *server_name);
int cs_envg_set_attack_t(cs_envg_t *self, double attack_t);
int cs_envg_set_decay_t(cs_envg_t *self, double decay_t);
int cs_envg_set_sustain_a(cs_envg_t *self, jack_default_audio_sample_t sustain_a);
int cs_envg_set_release_t(cs_envg_t *self, double release_t);

#endif
