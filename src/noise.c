#include <jack/jack.h>
#include <math.h>
#include <stdlib.h>
#include "cshellsynth/noise.h"
#include "cshellsynth/jclient.h"
#include "atomic.h"

const float A[] = { 0.02109238, 0.07113478, 0.68873558 }; // rescaled by (1+P)/(1-P)
const float P[] = { 0.3190,  0.7756,  0.9613  };

static inline float pink_random(float state[3]) {
    static const float RMI2 = 2.0 / ((float) RAND_MAX);
    static const float offset = 0.02109238 + 0.07113478 + 0.68873558;

    float temp = (float) random();
    state[0] = P[0] * (state[0] - temp) + temp;
    temp = (float) random();
    state[1] = P[1] * (state[1] - temp) + temp;
    temp = (float) random();
    state[2] = P[2] * (state[2] - temp) + temp;
    return ((A[0] * state[0] + A[1] * state[1] + A[2] * state[2]) * RMI2 - offset);
}

static int cs_noise_process(jack_nframes_t nframes, void *arg) {
    cs_noise_t *self = (cs_noise_t *) arg;
    float *out_buffer = (float *) jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    int kind = atomic_read(&self->kind);
    float offset = atomic_float_read(&self->offset);
    float amp = atomic_float_read(&self->amp);
    int i;
    for(i = 0; i < nframes; i++) {
	switch(kind) {
	case CS_WHITE:
	    out_buffer[i] = (((float) random()) / ((float) RAND_MAX)) * amp + offset;
	    break;
	case CS_PINK:
	    out_buffer[i] = pink_random(self->state) * amp + offset;
	    break;
	case CS_RED:
	    // not implemented
	default:
	    // unknown
	    return -1;
	}
    }
    return 0;
}

void cs_noise_set_kind(cs_noise_t *self, int kind) {
    atomic_set(&self->kind, kind);
}

void cs_noise_set_offset(cs_noise_t *self, float offset) {
    atomic_float_set(&self->offset, offset);
}

void cs_noise_set_amp(cs_noise_t *self, float amp) {
    atomic_float_set(&self->amp, amp);
}

int cs_noise_init(cs_noise_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = jclient_init((jclient_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }
    self->out_port = jack_port_register(self->client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(self->out_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return -1;
    }

    atomic_set(&self->kind, CS_WHITE);
    atomic_float_set(&self->amp, 1.0);
    atomic_float_set(&self->offset, 0.0);
    self->state[0] = 0.0;
    self->state[1] = 0.0;
    self->state[2] = 0.0;
    srandom((unsigned int) time(NULL));

    r = jack_set_process_callback(self->client, cs_noise_process, self);
    if(r != 0) {
	jclient_destroy((jclient_t *) self);
	return r;
    }
    r = jack_activate(self->client);
    if(r != 0) {
	jclient_destroy((jclient_t *) self);
	return r;
    }
    return 0;
}
