#include <ruby.h>
#include <jack/jack.h>
#include <math.h>
#include "jackruby.h"
#include <errno.h>

static VALUE cJEnvelopeGenerator;

enum jenvg_state {
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE,
    FINISHED
};

typedef struct jenvg_struct {
    jack_client_t *client;
    int closed;
    pthread_mutex_t lock;
    jack_port_t *ctl_port;
    jack_port_t *out_port;
    jack_default_audio_sample_t attack_t;
    jack_default_audio_sample_t decay_t;
    jack_default_audio_sample_t sustain_a;
    jack_default_audio_sample_t release_t;
    jack_nframes_t offset;
    enum jenvg_state state;
} jenvg_t;

static void jenvg_free(void *mem) {
    jenvg_t *cself = (jenvg_t *) mem;
    if(!cself->closed) {
	j_client_close(cself->client);
    }
    pthread_mutex_destroy(&cself->lock);
    xfree(cself);
}

static int jenvg_process(jack_nframes_t nframes, void *arg) {
    jenvg_t *cself = (jenvg_t *) arg;
    jack_default_audio_sample_t *ctl_buffer = (jack_default_audio_sample_t *) jack_port_get_buffer(cself->ctl_port, nframes);
    if(ctl_buffer == NULL) {
	return -1;
    }
    jack_default_audio_sample_t *out_buffer = (jack_default_audio_sample_t *) jack_port_get_buffer(cself->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    int r = j_mutex_lock(&cself->lock);
    {
	if(r != 0) {
	    return r;
	}
	int i;
	for(i = 0; i < nframes; i++) {
	    int ctl = ctl_buffer[i] == NAN ? 0 : ((int) ctl_buffer[i]);
	    switch(ctl) {
	    case 1:
		// attack event
		cself->offset = 0;
		cself->state = ATTACK;
		break;
	    case -1:
		// release event
		cself->offset = 0;
		cself->state = RELEASE;
	    }
	    jack_default_audio_sample_t offset;
	    switch(cself->state) {
	    case ATTACK:
		offset = (jack_default_audio_sample_t) cself->offset;
		if(offset < cself->attack_t) {
		    out_buffer[i] = offset / cself->attack_t;
		    cself->offset++;
		    break;
		} else {
		    cself->state = DECAY;
		    // fall through
		}
	    case DECAY:
		offset = ((jack_default_audio_sample_t) cself->offset) - cself->attack_t;
		if(offset < cself->decay_t) {
		    out_buffer[i] = (1.0f - offset / cself->decay_t) * (1.0f - cself->sustain_a) + cself->sustain_a;
		    cself->offset++;
		    break;
		} else {
		    cself->state = SUSTAIN;
		    // fall through
		}
	    case SUSTAIN:
		out_buffer[i] = cself->sustain_a;
		break;
	    case RELEASE:
		offset = (jack_default_audio_sample_t) cself->offset;
		if(offset < cself->release_t) {
		    out_buffer[i] = (1.0f - (offset / cself->release_t)) * cself->sustain_a;
		    cself->offset++;
		    break;
		} else {
		    cself->state = FINISHED;
		    // fall through
		}
	    case FINISHED:
	    default:
		out_buffer[i] = 0.0;
	    }
	}
    }
    r = j_mutex_unlock(&cself->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

static VALUE jenvg_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname, rattack_t, rdecay_t, rsustain_a, rrelease_t;
    char *name = "envg";
    double attack_t = 0.0, decay_t = 0.0, sustain_a = 1.0, release_t = 0.0;
    switch(rb_scan_args(argc, argv, "05", &rname, &rattack_t, &rdecay_t, &rsustain_a, &rrelease_t)) {
    case 5:
	release_t = NUM2DBL(rrelease_t);
    case 4:
	sustain_a = NUM2DBL(rsustain_a);
    case 3:
	decay_t = NUM2DBL(rdecay_t);
    case 2:
	attack_t = NUM2DBL(rattack_t);
    case 1:
	name = StringValueCStr(rname);
    }

    jenvg_t *cself = ALLOC(jenvg_t);
    jclient_init(name, 0, NULL, (jclient_t *) cself);

    cself->ctl_port = j_client_port_register(cself->client, "ctl", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(cself->ctl_port == NULL) {
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    cself->out_port = j_client_port_register(cself->client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(cself->out_port == NULL) {
	j_client_port_unregister(cself->client, cself->ctl_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    int r = jack_set_process_callback(cself->client, jenvg_process, cself);
    if(r != 0) {
	j_client_port_unregister(cself->client, cself->out_port);
	j_client_port_unregister(cself->client, cself->ctl_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Could not set process callback");
    }

    r = j_mutex_init(&cself->lock);
    if(r != 0) {
	j_client_port_unregister(cself->client, cself->out_port);
	j_client_port_unregister(cself->client, cself->ctl_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(rb_eRuntimeError, "Could not create pthread mutex: %s", strerror(errno));
    }

    jack_nframes_t sample_rate = jack_get_sample_rate(cself->client);
    cself->attack_t = attack_t * ((double) sample_rate);
    cself->decay_t = decay_t * ((double) sample_rate);
    cself->sustain_a = sustain_a;
    cself->release_t = release_t * ((double) sample_rate);
    cself->offset = 0;
    cself->state = FINISHED;

    r = j_client_activate(cself->client);
    if(r != 0) {
	pthread_mutex_destroy(&cself->lock);
	j_client_port_unregister(cself->client, cself->out_port);
	j_client_port_unregister(cself->client, cself->ctl_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(rb_eRuntimeError, "Could not create pthread mutex: %s", strerror(errno));
    }

    VALUE self = Data_Wrap_Struct(klass, 0, jenvg_free, cself);
    rb_iv_set(self, "@ctl", Data_Wrap_Struct(cJackPort, 0, fake_free, cself->ctl_port));
    rb_iv_set(self, "@out", Data_Wrap_Struct(cJackPort, 0, fake_free, cself->out_port));
    return self;
}

static VALUE jenvg_ctl(VALUE self) {
    return rb_iv_get(self, "@ctl");
}

static VALUE jenvg_set_ctl(VALUE self, VALUE port) {
    return jclient_connect(self, port, rb_iv_get(self, "@ctl"));
}

static VALUE jenvg_out(VALUE self) {
    return rb_iv_get(self, "@out");
}

static VALUE jenvg_set_attack_t(VALUE self, VALUE rattack_t) {
    jenvg_t *cself;
    Data_Get_Struct(self, jenvg_t, cself);
    jack_default_audio_sample_t attack_t = NUM2DBL(rattack_t) * ((double) jack_get_sample_rate(cself->client));
    int r = j_mutex_lock(&cself->lock);
    {
	if(r != 0) {
	    rb_raise(rb_eRuntimeError, "Could not acquire pthread mutex: %s", strerror(errno));
	}
	cself->attack_t = attack_t;
    }
    r = j_mutex_unlock(&cself->lock);
    if(r != 0) {
	rb_raise(rb_eRuntimeError, "Could not release pthread mutex: %s", strerror(errno));
    }
    return self;
}

static VALUE jenvg_set_decay_t(VALUE self, VALUE rdecay_t) {
    jenvg_t *cself;
    Data_Get_Struct(self, jenvg_t, cself);
    jack_default_audio_sample_t decay_t = NUM2DBL(rdecay_t) * ((double) jack_get_sample_rate(cself->client));
    int r = j_mutex_lock(&cself->lock);
    {
	if(r != 0) {
	    rb_raise(rb_eRuntimeError, "Could not acquire pthread mutex: %s", strerror(errno));
	}
	cself->decay_t = decay_t;
    }
    r = j_mutex_unlock(&cself->lock);
    if(r != 0) {
	rb_raise(rb_eRuntimeError, "Could not release pthread mutex: %s", strerror(errno));
    }
    return self;
}

static VALUE jenvg_set_sustain_a(VALUE self, VALUE rsustain_a) {
    jenvg_t *cself;
    Data_Get_Struct(self, jenvg_t, cself);
    jack_default_audio_sample_t sustain_a = NUM2DBL(rsustain_a);
    int r = j_mutex_lock(&cself->lock);
    {
	if(r != 0) {
	    rb_raise(rb_eRuntimeError, "Could not acquire pthread mutex: %s", strerror(errno));
	}
	cself->sustain_a = sustain_a;
    }
    r = j_mutex_unlock(&cself->lock);
    if(r != 0) {
	rb_raise(rb_eRuntimeError, "Could not release pthread mutex: %s", strerror(errno));
    }
    return self;
}

static VALUE jenvg_set_release_t(VALUE self, VALUE rrelease_t) {
    jenvg_t *cself;
    Data_Get_Struct(self, jenvg_t, cself);
    jack_default_audio_sample_t release_t = NUM2DBL(rrelease_t) * ((double) jack_get_sample_rate(cself->client));
    int r = j_mutex_lock(&cself->lock);
    {
	if(r != 0) {
	    rb_raise(rb_eRuntimeError, "Could not acquire pthread mutex: %s", strerror(errno));
	}
	cself->release_t = release_t;
    }
    r = j_mutex_unlock(&cself->lock);
    if(r != 0) {
	rb_raise(rb_eRuntimeError, "Could not release pthread mutex: %s", strerror(errno));
    }
    return self;
}

void Init_envelope_generator() {
    cJEnvelopeGenerator = rb_define_class_under(mJack, "EnvelopeGenerator", cJackClient);

    rb_define_singleton_method(cJEnvelopeGenerator, "new", jenvg_new, -1);
    rb_define_method(cJEnvelopeGenerator, "ctl", jenvg_ctl, 0);
    rb_define_method(cJEnvelopeGenerator, "ctl=", jenvg_set_ctl, 1);
    rb_define_method(cJEnvelopeGenerator, "out", jenvg_out, 0);
    rb_define_method(cJEnvelopeGenerator, "attack=", jenvg_set_attack_t, 1);
    rb_define_method(cJEnvelopeGenerator, "decay=", jenvg_set_decay_t, 1);
    rb_define_method(cJEnvelopeGenerator, "sustain=", jenvg_set_sustain_a, 1);
    rb_define_method(cJEnvelopeGenerator, "release=", jenvg_set_release_t, 1);
}
