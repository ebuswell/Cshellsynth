#include <ruby.h>
#include <jack/jack.h>
#include "jackruby.h"
#include <math.h>
#include <errno.h>

static VALUE cJInstrument;

typedef struct jinst_struct {
    jack_client_t *client;
    int closed;
    pthread_mutex_t lock;
    jack_port_t *out_port;
    jack_port_t *ctl_port;
    jack_default_audio_sample_t value;
    jack_default_audio_sample_t last_value;
} jinst_t;

static void jinst_free(void *mem) {
    jinst_t *cself = (jinst_t *) mem;
    if(!cself->closed) {
	j_client_close(cself->client);
    }
    pthread_mutex_destroy(&cself->lock);
    xfree(cself);
}

static int jinst_process(jack_nframes_t nframes, void *arg) {
    jinst_t *cself = (jinst_t *) arg;
    jack_default_audio_sample_t *out_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(cself->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    jack_default_audio_sample_t *ctl_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(cself->ctl_port, nframes);
    if(ctl_buffer == NULL) {
	return -1;
    }

    int r = j_mutex_lock(&cself->lock);
    {
	if(r != 0) {
	    return r;
	}
	int i;
	for(i = 0; i < nframes; i++) {
	    if(cself->value != cself->last_value) {
		if(cself->value == NAN) {
		    ctl_buffer[i] = -1.0f;
		} else {
		    ctl_buffer[i] = 1.0f;
		}
	    } else {
		ctl_buffer[i] = 0.0f;
	    }
	    cself->last_value = cself->value;
	    out_buffer[i] = cself->value;
	}
    }
    r = j_mutex_unlock(&cself->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

static VALUE jinst_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "inst";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }

    jinst_t *cself = ALLOC(jinst_t);
    jclient_init(name, 0, NULL, (jclient_t *) cself);

    cself->ctl_port = j_client_port_register(cself->client, "ctl", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
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

    int r = jack_set_process_callback(cself->client, jinst_process, cself);
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

    cself->value = NAN;
    cself->last_value = NAN;

    r = j_client_activate(cself->client);
    if(r != 0) {
	pthread_mutex_destroy(&cself->lock);
	j_client_port_unregister(cself->client, cself->out_port);
	j_client_port_unregister(cself->client, cself->ctl_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(rb_eRuntimeError, "Could not create pthread mutex: %s", strerror(errno));
    }

    VALUE self = Data_Wrap_Struct(klass, 0, jinst_free, cself);
    rb_iv_set(self, "@ctl", Data_Wrap_Struct(cJackPort, 0, fake_free, cself->ctl_port));
    rb_iv_set(self, "@out", Data_Wrap_Struct(cJackPort, 0, fake_free, cself->out_port));
    return self;
}

static VALUE jinst_out(VALUE self) {
    return rb_iv_get(self, "@out");
}

static VALUE jinst_ctl(VALUE self) {
    return rb_iv_get(self, "@ctl");
}

static VALUE jinst_play(VALUE self, VALUE rvalue) {
    jinst_t *cself;
    Data_Get_Struct(self, jinst_t, cself);
    jack_default_audio_sample_t value = (jack_default_audio_sample_t) NUM2DBL(rvalue);
    int r = j_mutex_lock(&cself->lock);
    {
	if(r != 0) {
	    rb_raise(rb_eRuntimeError, "Could not acquire pthread mutex: %s", strerror(errno));
	}
	cself->value = value;
    }
    r = j_mutex_unlock(&cself->lock);
    if(r != 0) {
	rb_raise(rb_eRuntimeError, "Could not release pthread mutex: %s", strerror(errno));
    }
    return self;
}

static VALUE jinst_stop(VALUE self) {
    jinst_t *cself;
    Data_Get_Struct(self, jinst_t, cself);
    int r = j_mutex_lock(&cself->lock);
    {
	if(r != 0) {
	    rb_raise(rb_eRuntimeError, "Could not acquire pthread mutex: %s", strerror(errno));
	}
	cself->value = NAN;
    }
    r = j_mutex_unlock(&cself->lock);
    if(r != 0) {
	rb_raise(rb_eRuntimeError, "Could not release pthread mutex: %s", strerror(errno));
    }
    return self;
}

void Init_instrument() {
    cJInstrument = rb_define_class_under(mJack, "Instrument", cJackClient);

    rb_define_singleton_method(cJInstrument, "new", jinst_new, -1);
    rb_define_method(cJInstrument, "out", jinst_out, 0);
    rb_define_method(cJInstrument, "ctl", jinst_ctl, 0);
    rb_define_method(cJInstrument, "play", jinst_play, 1);
    rb_define_method(cJInstrument, "stop", jinst_stop, 0);
}
