#include <ruby.h>
#include <jack/jack.h>
#include "jackruby.h"
#include <errno.h>

static VALUE cJTimer;

typedef struct jtimer_struct {
    jack_client_t *client;
    int closed;
    pthread_mutex_t lock;
    jack_port_t *ctl_port;
    double current;
    double step;
    double max;
} jtimer_t;

static int jtimer_process(jack_nframes_t nframes, void *arg) {
    jtimer_t *cself = (jtimer_t *) arg;
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
	    ctl_buffer[i] = (jack_default_audio_sample_t) cself->current;
	    cself->current += cself->step;
	    if(cself->current >= cself->max) {
		cself->current -= cself->max;
	    }
	}
    }
    r = j_mutex_unlock(&cself->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

static void jtimer_free(void *mem) {
    jtimer_t *cself = (jtimer_t *) mem;
    if(!cself->closed) {
	j_client_close(cself->client);
    }
    pthread_mutex_destroy(&cself->lock);
    xfree(cself);
}

static VALUE jtimer_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname, rbpm, rmax;
    char *name = "timer";
    double bpm = 144.0;
    double max = 4.0;
    switch(rb_scan_args(argc, argv, "03", &rname, &rbpm, &rmax)) {
    case 3:
	max = NUM2DBL(rmax);
    case 2:
	bpm = NUM2DBL(rbpm);
    case 1:
	name = StringValueCStr(rname);
    }

    jtimer_t *cself = ALLOC(jtimer_t);
    jclient_init(name, 0, NULL, (jclient_t *) cself);

    cself->ctl_port = j_client_port_register(cself->client, "ctl", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(cself->ctl_port == NULL) {
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    int r = j_mutex_init(&cself->lock);
    if(r != 0) {
	j_client_port_unregister(cself->client, cself->ctl_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(rb_eRuntimeError, "Could not create pthread mutex: %s", strerror(errno));
    }

    r = jack_set_process_callback(cself->client, jtimer_process, cself);
    if(r != 0) {
	pthread_mutex_destroy(&cself->lock);
	j_client_port_unregister(cself->client, cself->ctl_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Could not set process callback");
    }

    cself->step = bpm / (60.0 * jack_get_sample_rate(cself->client));
    cself->max = max;
    cself->current = 0.0;

    r = j_client_activate(cself->client);
    if(r != 0) {
	pthread_mutex_destroy(&cself->lock);
	j_client_port_unregister(cself->client, cself->ctl_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Could not set process callback");
    }

    VALUE self = Data_Wrap_Struct(klass, 0, jtimer_free, cself);
    rb_iv_set(self, "@ctl", Data_Wrap_Struct(cJackPort, 0, fake_free, cself->ctl_port));
    return self;
}

static VALUE jtimer_ctl(VALUE self) {
    return rb_iv_get(self, "@ctl");
}

static VALUE jtimer_set_bpm(VALUE self, VALUE bpm) {
    jtimer_t *cself;
    Data_Get_Struct(self, jtimer_t, cself);
    int r = j_mutex_lock(&cself->lock);
    {
	if(r != 0) {
	    rb_raise(rb_eRuntimeError, "Could not acquire pthread mutex: %s", strerror(errno));
	}
	cself->step = NUM2DBL(bpm) / (60.0 * jack_get_sample_rate(cself->client));
    }
    r = j_mutex_unlock(&cself->lock);
    if(r != 0) {
	rb_raise(rb_eRuntimeError, "Could not release pthread mutex: %s", strerror(errno));
    }
    return self;
}

static VALUE jtimer_set_signature(VALUE self, VALUE signature) {
    jtimer_t *cself;
    Data_Get_Struct(self, jtimer_t, cself);
    int r = j_mutex_lock(&cself->lock);
    {
	if(r != 0) {
	    rb_raise(rb_eRuntimeError, "Could not acquire pthread mutex: %s", strerror(errno));
	}
	cself->max = NUM2DBL(signature);
    }
    r = j_mutex_unlock(&cself->lock);
    if(r != 0) {
	rb_raise(rb_eRuntimeError, "Could not release pthread mutex: %s", strerror(errno));
    }
    return self;
}

void Init_timer() {
    cJTimer = rb_define_class_under(mJack, "Timer", cJackClient);
    
    rb_define_singleton_method(cJTimer, "new", jtimer_new, -1);
    rb_define_method(cJTimer, "ctl", jtimer_ctl, 0);
    rb_define_method(cJTimer, "bpm=", jtimer_set_bpm, 1);
    rb_define_method(cJTimer, "signature=", jtimer_set_signature, 1);
}
