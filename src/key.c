#include <jack/jack.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "jclient.h"
#include "key.h"

const jack_default_audio_sample_t CS_MAJOR_TUNING[] = {
    1.0f,
    9.0f/8.0f,
    5.0f/4.0f,
    4.0f/3.0f,
    3.0f/2.0f,
    (4.0f/3.0f)*(5.0f/4.0f),
    (3.0f/2.0f)*(5.0f/4.0f)
};

const jack_default_audio_sample_t CS_MINOR_TUNING[] = {
    1.0f,
    9.0f/8.0f,
    6.0f/5.0f,
    4.0f/3.0f,
    3.0f/2.0f,
    (4.0f/3.0f)*(6.0f/5.0f),
    (3.0f/2.0f)*(6.0f/5.0f)
};

const jack_default_audio_sample_t CS_PYTHAGOREAN_TUNING[] = {
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

static inline jack_default_audio_sample_t cs_key_note2freq_nolock(cs_key_t *self, jack_default_audio_sample_t note) {
    if(isnanf(note)) {
	return NAN;
    }
    if(self->tuning == CS_EQUAL_TUNING) {
	return self->root * powf(2.0f, (note/12.0f));
    } else {
	int tuning_length = self->tuning_length;
	jack_default_audio_sample_t f = note;
	int n = floorf(f);
	f -= (jack_default_audio_sample_t) n;
	int m = n % tuning_length;
	int e = n / tuning_length;
	if(m < 0) {
	    e -= 1;
	    m = tuning_length + m;
	}
	jack_default_audio_sample_t freq = self->tuning[m];
	if(f != 0.0f) {
	    if(m == (tuning_length - 1)) {
		freq *= powf(2.0/self->tuning[m], f);
	    } else {
		freq *= powf(self->tuning[m + 1]/self->tuning[m], f);
	    }
	}
	if(e >= 0) {
	    return ((freq * self->root) * (1 << e));
	} else {
	    e = -e;
	    return ((freq * self->root) / (1 << e));
	}
    }
}

static int cs_key_process(jack_nframes_t nframes, void *arg) {
    cs_key_t *self = (cs_key_t *) arg;

    jack_default_audio_sample_t *note_buffer;
    jack_default_audio_sample_t *freq_buffer = (jack_default_audio_sample_t *) jack_port_get_buffer(self->freq_port, nframes);
    if(freq_buffer == NULL) {
	return -1;
    }

    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	if(isnanf(self->note)) {
	    note_buffer = (jack_default_audio_sample_t *) jack_port_get_buffer(self->note_port, nframes);
	    if(note_buffer == NULL) {
		return -1;
	    }
	    int i;
	    for(i = 0; i < nframes; i++) {
		freq_buffer[i] = cs_key_note2freq_nolock(self, note_buffer[i]);
	    }
	} else {
	    int i;
	    jack_default_audio_sample_t freq = cs_key_note2freq_nolock(self, self->note);
	    for(i = 0; i < nframes; i++) {
		freq_buffer[i] = freq;
	    }
	}
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

jack_default_audio_sample_t cs_key_note2freq(cs_key_t *self, jack_default_audio_sample_t note) {
    jack_default_audio_sample_t freq;
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return NAN;
	}
	freq = cs_key_note2freq_nolock(self, note);
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return NAN;
    }
    return freq;
}

int cs_key_set_tuning(cs_key_t *self, const jack_default_audio_sample_t *tuning, size_t tuning_length) {
    jack_default_audio_sample_t *tuning_cpy;
    if(tuning != CS_EQUAL_TUNING) {
	tuning_cpy = malloc(tuning_length * sizeof(jack_default_audio_sample_t));
	memcpy(tuning_cpy, tuning, tuning_length * sizeof(jack_default_audio_sample_t));
    }
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	if((self->tuning != NULL)
	   && (self->tuning != CS_EQUAL_TUNING)) {
	    free(self->tuning);
	}
	if(tuning == CS_EQUAL_TUNING) {
	    self->tuning_length = tuning_length;
	    self->tuning = CS_EQUAL_TUNING;
	} else {
	    self->tuning_length = tuning_length;
	    self->tuning = tuning_cpy;
	}
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

int cs_key_set_root(cs_key_t *self, jack_default_audio_sample_t root) {
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	self->root = root;
    }
    r = pthread_mutex_unlock(&self->lock);
}

int cs_key_set_note(cs_key_t *self, jack_default_audio_sample_t note) {
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	self->note = note;
    }
    r = pthread_mutex_unlock(&self->lock);
}

int cs_key_init(cs_key_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = jclient_locking_init((jclient_locking_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->note_port = jack_port_register(self->client, "note", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->note_port == NULL) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return r;
    }

    self->freq_port = jack_port_register(self->client, "freq", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(self->freq_port == NULL) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return r;
    }

    self->tuning_length = CS_EQUAL_TUNING_SIZE;
    self->tuning = CS_EQUAL_TUNING;
    self->root = CS_C;
    self->note = NAN;

    r = jack_set_process_callback(self->client, cs_key_process, self);
    if(r != 0) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return r;
    }

    r = jack_activate(self->client);
    if(r != 0) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return r;
    }

    return 0;
}
