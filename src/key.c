#include <jack/jack.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "cshellsynth/jclient.h"
#include "cshellsynth/key.h"
#include "atomic-float.h"
#include "atomic-ptr.h"

const float CS_MAJOR_TUNING[] = {
    1.0f,
    9.0f/8.0f,
    5.0f/4.0f,
    4.0f/3.0f,
    3.0f/2.0f,
    (4.0f/3.0f)*(5.0f/4.0f),
    (3.0f/2.0f)*(5.0f/4.0f)
};

const cs_key_tuning_t cs_major_tuning = {
    CS_MAJOR_TUNING_LENGTH,
    CS_MAJOR_TUNING
};

const float CS_MINOR_TUNING[] = {
    1.0f,
    9.0f/8.0f,
    6.0f/5.0f,
    4.0f/3.0f,
    3.0f/2.0f,
    (4.0f/3.0f)*(6.0f/5.0f),
    (3.0f/2.0f)*(6.0f/5.0f)
};

const cs_key_tuning_t cs_minor_tuning = {
    CS_MINOR_TUNING_LENGTH,
    CS_MINOR_TUNING
};

const float CS_PYTHAGOREAN_TUNING[] = {
    1.0f,
    256.0f/243.0f,
    9.0f/8.0f,
    32.0f/27.0f,
    81.0f/64.0f,
    4.0f/3.0f,
    1024.0f/729.0f,
    32.0f/2.0f,
    128.0f/81.0f,
    27.0f/16.0f,
    16.0f/9.0f,
    243.0f/128.0f
};

const cs_key_tuning_t cs_pythagorean_tuning = {
    CS_PYTHAGOREAN_TUNING_LENGTH,
    CS_PYTHAGOREAN_TUNING
};

#define cs_equal_tuning ((cs_key_tuning_t *) -1)

static inline float cs_key_note2freq_param(float note, float root, cs_key_tuning_t *tuning) {
    if(isnanf(note)) {
	return NAN;
    }
    if((tuning == cs_equal_tuning)
       || (tuning->tuning == CS_EQUAL_TUNING)) {
	return root * powf(2.0f, (note/12.0f));
    } else {
	int tuning_length = tuning->tuning_length;
	float f = note;
	float n_f = floorf(f);
	f -= n_f;
	int n = (int) n_f;
	int m = n % tuning_length;
	int e = n / tuning_length;
	if(m < 0) {
	    e--;
	    m = tuning_length + m;
	}
	float freq = tuning->tuning[m];
	if(f != 0.0f) {
	    if(m == (tuning_length - 1)) {
		freq *= powf(2.0/tuning->tuning[m], f);
	    } else {
		freq *= powf(tuning->tuning[m + 1]/tuning->tuning[m], f);
	    }
	}
	if(e >= 0) {
	    return ((freq * root) * (1 << e));
	} else {
	    e = -e;
	    return ((freq * root) / (1 << e));
	}
    }
}

static int cs_key_process(jack_nframes_t nframes, void *arg) {
    cs_key_t *self = (cs_key_t *) arg;

    float *note_buffer;
    float *freq_buffer = (float *) jack_port_get_buffer(self->freq_port, nframes);
    if(freq_buffer == NULL) {
	return -1;
    }

    float note = atomic_float_read(&self->note);
    float root = atomic_float_read(&self->root);
    cs_key_tuning_t *tuning = atomic_ptr_read(&self->tuning);
    if(isnanf(note)) {
	note_buffer = (float *) jack_port_get_buffer(self->note_port, nframes);
	if(note_buffer == NULL) {
	    return -1;
	}
	int i;
	for(i = 0; i < nframes; i++) {
	    freq_buffer[i] = cs_key_note2freq_param(note_buffer[i], root, tuning);
	}
    } else {
	int i;
	float freq = cs_key_note2freq_param(note, root, tuning);
	for(i = 0; i < nframes; i++) {
	    freq_buffer[i] = freq;
	}
    }
    return 0;
}

float cs_key_note2freq(cs_key_t *self, float note) {
    float root = atomic_float_read(&self->root);
    cs_key_tuning_t *tuning = atomic_ptr_read(&self->tuning);
    return cs_key_note2freq_param(note, root, tuning);
}

void cs_key_free_tuning(cs_key_tuning_t *tuning) {
    if((tuning != &cs_major_tuning)
       && (tuning != cs_equal_tuning)
       && (tuning != &cs_minor_tuning)
       && (tuning != &cs_pythagorean_tuning)) {
	if((tuning->tuning != CS_MAJOR_TUNING)
	   && (tuning->tuning != CS_EQUAL_TUNING)
	   && (tuning->tuning != CS_MINOR_TUNING)
	   && (tuning->tuning != CS_PYTHAGOREAN_TUNING)) {
	    free((void *) tuning->tuning);
	}
	free(tuning);
    }
}

void cs_key_set_tuning(cs_key_t *self, const float *tuning, size_t tuning_length) {
    cs_key_tuning_t *oldtuning;
    if((tuning == CS_EQUAL_TUNING)
       && (tuning_length = CS_EQUAL_TUNING_LENGTH)) {
	oldtuning = atomic_ptr_xchg(&self->tuning, cs_equal_tuning);
    } else if((tuning == CS_MAJOR_TUNING)
	      && (tuning_length == CS_MAJOR_TUNING_LENGTH)) {
	oldtuning = atomic_ptr_xchg(&self->tuning, &cs_major_tuning);
    } else if((tuning == CS_MINOR_TUNING)
	      && (tuning_length == CS_MINOR_TUNING_LENGTH)) {
	oldtuning = atomic_ptr_xchg(&self->tuning, &cs_minor_tuning);
    } else if((tuning == CS_PYTHAGOREAN_TUNING)
	      && (tuning_length == CS_PYTHAGOREAN_TUNING_LENGTH)) {
	oldtuning = atomic_ptr_xchg(&self->tuning, &cs_pythagorean_tuning);
    } else {
	cs_key_tuning_t *newtuning = malloc(sizeof(cs_key_tuning_t));
	newtuning->tuning_length = tuning_length;
	if((tuning != CS_MAJOR_TUNING)
	   && (tuning != CS_MINOR_TUNING)
	   && (tuning != CS_PYTHAGOREAN_TUNING)
	   && (tuning != CS_EQUAL_TUNING)) {
	    newtuning->tuning = malloc(sizeof(float) * tuning_length);
	    memcpy((void *) newtuning->tuning, tuning, sizeof(float) * tuning_length);
	} else {
	    newtuning->tuning = tuning;
	}
    }
    if(oldtuning != NULL) {
	cs_key_free_tuning(oldtuning);
    }
}

void cs_key_set_root(cs_key_t *self, float root) {
    atomic_float_set(&self->root, root);
}

void cs_key_set_note(cs_key_t *self, float note) {
    atomic_float_set(&self->note, note);
}

int cs_key_destroy(cs_key_t *self) {
    int r = jclient_destroy((jclient_t *) self);
    cs_key_tuning_t *oldtuning = atomic_ptr_xchg(&self->tuning, NULL);
    if(oldtuning != NULL) {
	cs_key_free_tuning(oldtuning);
    }
    return r;
}

int cs_key_init(cs_key_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = jclient_init((jclient_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->note_port = jack_port_register(self->client, "note", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->note_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return r;
    }

    self->freq_port = jack_port_register(self->client, "freq", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(self->freq_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return r;
    }

    atomic_ptr_set(&self->tuning, cs_equal_tuning);
    atomic_float_set(&self->root, CS_C);
    atomic_float_set(&self->note, NAN);

    r = jack_set_process_callback(self->client, cs_key_process, self);
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
