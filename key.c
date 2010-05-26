#include <ruby.h>
#include <jack/jack.h>
#include "jackruby.h"
#include <math.h>
#include <errno.h>

static VALUE cJKey;

typedef struct jkey_struct {
    jack_client_t *client;
    int closed;
    pthread_mutex_t lock;
    JackProcessCallback cb;
    jack_port_t *note_port;
    jack_port_t *freq_port;
    jack_default_audio_sample_t *tuning;
    size_t tuning_length;
    jack_default_audio_sample_t root;
} jkey_t;

static void jkey_free(void *mem) {
    jkey_t *cself = (jkey_t *) mem;
    if(!cself->closed) {
	j_client_close(cself->client);
    }
    pthread_mutex_destroy(&cself->lock);
    if(cself->tuning != NULL) {
	free(cself->tuning);
    }
    jclient_free(mem);
}

static int jkey_process(jack_nframes_t nframes, void *arg) {
    jkey_t *cself = (jkey_t *) arg;
    int r = j_mutex_lock(&cself->lock);
    {
	if(r != 0) {
	    return r;
	}
	cself->cb(nframes, arg);
    }
    r = j_mutex_unlock(&cself->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

static int jkey_process_array(jack_nframes_t nframes, void *arg) {
    jkey_t *jkey = (jkey_t *) arg;
    jack_default_audio_sample_t *note_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(jkey->note_port, nframes);
    if(note_buffer == NULL) {
	return -1;
    }
    jack_default_audio_sample_t *freq_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(jkey->freq_port, nframes);
    if(freq_buffer == NULL) {
	return -1;
    }
    int i;
    int tuning_length = jkey->tuning_length;
    jack_default_audio_sample_t *tuning = jkey->tuning;
    jack_default_audio_sample_t root = jkey->root;
    for(i = 0; i < nframes; i++) {
	jack_default_audio_sample_t f = note_buffer[i];
	int n = floor(f);
	f -= (jack_default_audio_sample_t) n;
	int m = n % tuning_length;
	int e = n / tuning_length;
	if(m < 0) {
	    e -= 1;
	    m = tuning_length + m;
	}
	jack_default_audio_sample_t note = tuning[m];
	if(f != 0.0) {
	    if(m == (tuning_length - 1)) {
		note *= pow(2.0/tuning[m], f);
	    } else {
		note *= pow(tuning[m + 1]/tuning[m], f);
	    }
	}
	if(e >= 0) {
	    freq_buffer[i] = (note * root) * (1 << e);
	} else {
	    e = -e;
	    freq_buffer[i] = (note * root) / (1 << e);
	}
    }
    return 0;
}

static int jkey_process_equal(jack_nframes_t nframes, void *arg) {
    jkey_t *jkey = (jkey_t *) arg;
    jack_default_audio_sample_t *note_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(jkey->note_port, nframes);
    if(note_buffer == NULL) {
	return -1;
    }
    jack_default_audio_sample_t *freq_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(jkey->freq_port, nframes);
    if(freq_buffer == NULL) {
	return -1;
    }
    int i;
    jack_default_audio_sample_t root = jkey->root;
    for(i = 0; i < nframes; i++) {
	jack_default_audio_sample_t n = note_buffer[i];
	freq_buffer[i] = root * pow(2.0f, (n/12.0f));
    }
    return 0;
}

static VALUE jkey_set_tuning(VALUE self, VALUE rtuning) {
    jkey_t *key;
    Data_Get_Struct(self, jkey_t, key);
    if(TYPE(rtuning) == T_ARRAY) {
	size_t tuning_length = RARRAY_LEN(rtuning);
	jack_default_audio_sample_t *ptr = malloc(sizeof(jack_default_audio_sample_t) * tuning_length);
	VALUE *ptr2 = RARRAY_PTR(rtuning);
	int i;
	for(i = tuning_length; i > 0; i--) {
	    // memory leak if NUM2DBL throws!
	    *ptr++ = (jack_default_audio_sample_t) NUM2DBL(*ptr2++);
	}
	int r = j_mutex_lock(&key->lock);
	{
	    if(r != 0) {
		free(ptr);
		rb_raise(rb_eRuntimeError, "Could not acquire pthread mutex: %s", strerror(errno));
	    }
	    if(key->tuning != NULL) {
		free(key->tuning);
	    }
	    key->cb = jkey_process_array;
	    key->tuning_length = tuning_length;
	    key->tuning = ptr;
	}
	j_mutex_unlock(&key->lock);
    } else {
	char *ctuning = StringValueCStr(rtuning);
	if(strcmp(ctuning, "major") == 0) {
	    size_t tuning_length = 7;
	    jack_default_audio_sample_t *tuning = malloc(sizeof(jack_default_audio_sample_t) * tuning_length);
	    tuning[0] = 1.0f;
	    tuning[1] = 9.0f/8.0f;
	    tuning[2] = 5.0f/4.0f;
	    tuning[3] = 4.0f/3.0f;
	    tuning[4] = 3.0f/2.0f;
	    tuning[5] = (4.0f/3.0f)*(5.0f/4.0f);
	    tuning[6] = (3.0f/2.0f)*(5.0f/4.0f);
	    int r = j_mutex_lock(&key->lock);
	    {
		if(r != 0) {
		    free(tuning);
		    rb_raise(rb_eRuntimeError, "Could not acquire pthread mutex: %s", strerror(errno));
		}
		if(key->tuning != NULL) {
		    free(key->tuning);
		}
		key->cb = jkey_process_array;
		key->tuning_length = tuning_length;
		key->tuning = tuning;
	    }
	    j_mutex_unlock(&key->lock);
	} else if(strcmp(ctuning, "minor") == 0) {
	    size_t tuning_length = 7;
	    jack_default_audio_sample_t *tuning = malloc(sizeof(jack_default_audio_sample_t) * tuning_length);
	    tuning[0] = 1.0f;
	    tuning[1] = 9.0f/8.0f;
	    tuning[2] = 6.0f/5.0f;
	    tuning[3] = 4.0f/3.0f;
	    tuning[4] = 3.0f/2.0f;
	    tuning[5] = (4.0f/3.0f)*(6.0f/5.0f);
	    tuning[6] = (3.0f/2.0f)*(6.0f/5.0f);
	    int r = j_mutex_lock(&key->lock);
	    {
		if(r != 0) {
		    free(tuning);
		    rb_raise(rb_eRuntimeError, "Could not acquire pthread mutex: %s", strerror(errno));
		}
		if(key->tuning != NULL) {
		    free(key->tuning);
		}
		key->cb = jkey_process_array;
		key->tuning_length = tuning_length;
		key->tuning = tuning;
	    }
	    j_mutex_unlock(&key->lock);
	} else if(strcmp(ctuning, "equal") == 0) {
	    size_t tuning_length = 12;
	    jack_default_audio_sample_t *tuning = malloc(sizeof(jack_default_audio_sample_t) * tuning_length);
	    tuning[0] = 1.0;
	    tuning[1] = pow(2.0f, (1.0f/12.0f));
	    tuning[2] = pow(2.0f, (2.0f/12.0f));
	    tuning[3] = pow(2.0f, (3.0f/12.0f));
	    tuning[4] = pow(2.0f, (4.0f/12.0f));
	    tuning[5] = pow(2.0f, (5.0f/12.0f));
	    tuning[6] = pow(2.0f, (6.0f/12.0f));
	    tuning[7] = pow(2.0f, (7.0f/12.0f));
	    tuning[8] = pow(2.0f, (8.0f/12.0f));
	    tuning[9] = pow(2.0f, (9.0f/12.0f));
	    tuning[10] = pow(2.0f, (10.0f/12.0f));
	    tuning[11] = pow(2.0f, (11.0f/12.0f));
	    int r = j_mutex_lock(&key->lock);
	    {
		if(r != 0) {
		    free(tuning);
		    rb_raise(rb_eRuntimeError, "Could not acquire pthread mutex: %s", strerror(errno));
		}
		if(key->tuning != NULL) {
		    free(key->tuning);
		}
		key->cb = jkey_process_equal;
		key->tuning_length = tuning_length;
		key->tuning = tuning;
	    }
	    j_mutex_unlock(&key->lock);
	} else if(strcmp(ctuning, "pythagorean") == 0) {
	    size_t tuning_length = 12;
	    jack_default_audio_sample_t *tuning = malloc(sizeof(jack_default_audio_sample_t) * tuning_length);
	    tuning[0] = 1.0f;
	    tuning[1] = 256.0f/243.0f;
	    tuning[2] = 9.0f/8.0f;
	    tuning[3] = 32.0f/27.0f;
	    tuning[4] = 81.0f/64.0f;
	    tuning[5] = 4.0f/3.0f;
	    tuning[6] = 1024.0f/729.0f;
	    tuning[7] = 3.0f/2.0f;
	    tuning[8] = 128.0f/81.0f;
	    tuning[9] = 27.0f/16.0f;
	    tuning[10] = 16.0f/9.0f;
	    tuning[11] = 243.0f/128.0f;
	    int r = j_mutex_lock(&key->lock);
	    {
		if(r != 0) {
		    free(tuning);
		    rb_raise(rb_eRuntimeError, "Could not acquire pthread mutex: %s", strerror(errno));
		}
		if(key->tuning != NULL) {
		    free(key->tuning);
		}
		key->cb = jkey_process_array;
		key->tuning_length = tuning_length;
		key->tuning = tuning;
	    }
	    j_mutex_unlock(&key->lock);
	} else {
	    rb_raise(rb_eArgError, "Invalid tuning");
	}
    }
    return self;
}

static VALUE jkey_set_root(VALUE self, VALUE root) {
    jkey_t *key;
    char *croot;
    Data_Get_Struct(self, jkey_t, key);
    switch(TYPE(root)) {
    case T_FLOAT:
    case T_FIXNUM:
    case T_BIGNUM:
	j_mutex_lock(&key->lock);
	key->root = (jack_default_audio_sample_t) NUM2DBL(root);
	j_mutex_unlock(&key->lock);
	break;
    default:
	croot = StringValueCStr(root);
	if(strcmp(croot, "A") == 0) {
	    j_mutex_lock(&key->lock);
	    key->root = 220.0f;
	    j_mutex_unlock(&key->lock);
	} else if((strcmp(croot, "A#") == 0) || ((strcmp(croot, "Bb") == 0))) {
	    j_mutex_lock(&key->lock);
	    key->root = 220.0f * pow(2.0f, 1.0f/12.0f);
	    j_mutex_unlock(&key->lock);
	} else if((strcmp(croot, "B") == 0) || ((strcmp(croot, "Cb") == 0))) {
	    j_mutex_lock(&key->lock);
	    key->root = 220.0f * pow(2.0f, 2.0f/12.0f);
	    j_mutex_unlock(&key->lock);
	} else if((strcmp(croot, "C") == 0) || ((strcmp(croot, "B#") == 0))) {
	    j_mutex_lock(&key->lock);
	    key->root = 220.0f * pow(2.0f, 3.0f/12.0f);
	    j_mutex_unlock(&key->lock);
	} else if((strcmp(croot, "C#") == 0) || ((strcmp(croot, "Db") == 0))) {
	    j_mutex_lock(&key->lock);
	    key->root = 220.0f * pow(2.0f, 4.0f/12.0f);
	    j_mutex_unlock(&key->lock);
	} else if(strcmp(croot, "D") == 0) {
	    j_mutex_lock(&key->lock);
	    key->root = 220.0f * pow(2.0f, 5.0f/12.0f);
	    j_mutex_unlock(&key->lock);
	} else if((strcmp(croot, "D#") == 0) || ((strcmp(croot, "Eb") == 0))) {
	    j_mutex_lock(&key->lock);
	    key->root = 220.0f * pow(2.0f, 6.0f/12.0f);
	    j_mutex_unlock(&key->lock);
	} else if((strcmp(croot, "E") == 0) || ((strcmp(croot, "Fb") == 0))) {
	    j_mutex_lock(&key->lock);
	    key->root = 220.0f * pow(2.0f, 7.0f/12.0f);
	    j_mutex_unlock(&key->lock);
	} else if((strcmp(croot, "F") == 0) || ((strcmp(croot, "E#") == 0))) {
	    j_mutex_lock(&key->lock);
	    key->root = 220.0f * pow(2.0f, 8.0f/12.0f);
	    j_mutex_unlock(&key->lock);
	} else if((strcmp(croot, "F#") == 0) || ((strcmp(croot, "Gb") == 0))) {
	    j_mutex_lock(&key->lock);
	    key->root = 220.0f * pow(2.0f, 9.0f/12.0f);
	    j_mutex_unlock(&key->lock);
	} else if(strcmp(croot, "G") == 0) {
	    j_mutex_lock(&key->lock);
	    key->root = 220.0f * pow(2.0f, 10.0f/12.0f);
	    j_mutex_unlock(&key->lock);
	} else if((strcmp(croot, "G#") == 0) || ((strcmp(croot, "Ab") == 0))) {
	    j_mutex_lock(&key->lock);
	    key->root = 220.0f * pow(2.0f, 11.0f/12.0f);
	    j_mutex_unlock(&key->lock);
	} else {
	    rb_raise(rb_eArgError, "Invalid root");
	}
    }
    return self;
}

static VALUE jkey_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname, rroot, rtuning;
    char *name = "key";
    if(rb_scan_args(argc, argv, "03", &rname, &rroot, &rtuning)) {
	name = StringValueCStr(rname);
    }

    jkey_t *cself = ALLOC(jkey_t);
    jclient_init(name, 0, NULL, (jclient_t *) cself);

    cself->note_port = j_client_port_register(cself->client, "note", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(cself->note_port == NULL) {
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    cself->freq_port = j_client_port_register(cself->client, "freq", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(cself->freq_port == NULL) {
	j_client_port_unregister(cself->client, cself->note_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    int r = j_mutex_init(&cself->lock);
    if(r != 0) {
	j_client_port_unregister(cself->client, cself->note_port);
	j_client_port_unregister(cself->client, cself->freq_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(rb_eRuntimeError, "Could not create pthread mutex: %s", strerror(errno));
    }

    r = jack_set_process_callback(cself->client, jkey_process, cself);
    if(r != 0) {
	j_client_port_unregister(cself->client, cself->note_port);
	j_client_port_unregister(cself->client, cself->freq_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Could not set process callback");
    }

    cself->cb = jkey_process_array;
    cself->tuning = NULL;

    VALUE self = Data_Wrap_Struct(klass, 0, jkey_free, cself);
    rb_iv_set(self, "@note", Data_Wrap_Struct(cJackPort, 0, fake_free, cself->note_port));
    rb_iv_set(self, "@freq", Data_Wrap_Struct(cJackPort, 0, fake_free, cself->freq_port));
    if(NIL_P(rroot)) {
	rroot = rb_str_new_cstr("C");
    }
    if(NIL_P(rtuning)) {
	rtuning = rb_str_new_cstr("equal");
    }
    jkey_set_tuning(self, rtuning);
    jkey_set_root(self, rroot);

    r = j_client_activate(cself->client);
    if(r != 0) {
	rb_raise(eJackFailure, "Overall operation failed"); // hopefully gc will clean up?
    }

    return self;
}

static VALUE jkey_note(VALUE self) {
    return rb_iv_get(self, "@note");
}

static VALUE jkey_set_note(VALUE self, VALUE port) {
    return jclient_connect(self, port, rb_iv_get(self, "@note"));
}

static VALUE jkey_freq(VALUE self) {
    return rb_iv_get(self, "@freq");
}

void Init_key() {
    cJKey = rb_define_class_under(mJack, "Key", cJackClient);
    
    rb_define_singleton_method(cJKey, "new", jkey_new, -1);
    rb_define_method(cJKey, "note", jkey_note, 0);
    rb_define_method(cJKey, "note=", jkey_set_note, 1);
    rb_define_method(cJKey, "freq", jkey_freq, 0);
    rb_define_method(cJKey, "tuning=", jkey_set_tuning, 1);
    rb_define_method(cJKey, "root=", jkey_set_root, 1);
}
