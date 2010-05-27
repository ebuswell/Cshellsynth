#include <jack/jack.h>
#include "jclient.h"
#include "key.h"

const jack_default_audio_sample_t *CS_MAJOR_TUNING = {
    1.0f,
    9.0f/8.0f,
    5.0f/4.0f,
    4.0f/3.0f,
    3.0f/2.0f,
    (4.0f/3.0f)*(5.0f/4.0f),
    (3.0f/2.0f)*(5.0f/4.0f)
};

const jack_default_audio_sample_t *CS_MINOR_TUNING = {
    1.0f,
    9.0f/8.0f,
    6.0f/5.0f,
    4.0f/3.0f,
    3.0f/2.0f,
    (4.0f/3.0f)*(6.0f/5.0f),
    (3.0f/2.0f)*(6.0f/5.0f)
};

const jack_default_audio_type *CS_PYTHAGOREAN_TUNING = {
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

static int cs_key_process(jack_nframes_t nframes, void *arg) {
    cs_key_t *self = (cs_key_t *) arg;

    jack_default_audio_sample_t *note_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(self->note_port, nframes);
    if(note_buffer == NULL) {
	return -1;
    }

    jack_default_audio_sample_t *freq_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(self->freq_port, nframes);
    if(freq_buffer == NULL) {
	return -1;
    }

    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	int i;
	for(i = 0; i < nframes; i++) {
	    freq_buffer[i] = cs_key_note2freq(self, note_buffer[i]);
	}
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

jack_default_audio_sample_t cs_key_note2freq(cs_key_t *self, jack_default_audio_sample_t note) {
    if(self->tuning == CS_EQUAL_TUNING) {
	return self->root * powf(2.0f, (note/12.0f));
    } else {
	jack_default_audio_sample_t f = note;
	int n = floor(f);
	f -= (jack_default_audio_sample_t) n;
	int m = n % tuning_length;
	int e = n / tuning_length;
	if(m < 0) {
	    e -= 1;
	    m = tuning_length + m;
	}
	jack_default_audio_sample_t freq = tuning[m];
	if(f != 0.0f) {
	    if(m == (tuning_length - 1)) {
		freq *= powf(2.0/tuning[m], f);
	    } else {
		freq *= powf(tuning[m + 1]/tuning[m], f);
	    }
	}
	if(e >= 0) {
	    return (freq * root) * (1 << e);
	} else {
	    e = -e;
	    return (freq * root) / (1 << e);
	}
    }
}

int cs_key_set_tuning(cs_key_t *self, jack_default_audio_sample_t *tuning, size_t tuning_length) {
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
	    key->tuning_length = tuning_length;
	    key->tuning = CS_EQUAL_TUNING;
	} else {
	    key->tuning_length = tuning_length;
	    key->tuning = tuning_cpy;
	}
    }
    r = pthread_mutex_unlock(&key->lock);
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

int cs_key_init(cs_key_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = jclient_locking_init((jclient_locking_t *) self);
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

    self->tuning_length = CS_EQUAL_TUNING_LENGTH;
    self->tuning = CS_EQUAL_TUNING;
    self->root = CS_C;

    r = jack_set_process_callback(cself->client, cs_key_process, cself);
    if(r != 0) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return r;
    }
    return 0;
}
