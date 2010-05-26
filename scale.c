#include <ruby.h>
#include <jack/jack.h>
#include "jackruby.h"
#include <errno.h>

static VALUE cJScale;

typedef struct jscale_struct {
    jack_client_t *client;
    int closed;
    pthread_mutex_t lock;
    jack_port_t *in_port;
    jack_port_t *out_port;
    jack_default_audio_sample_t scale;
} jscale_t;

static void jscale_free(void *mem) {
    jscale_t *cself = (jscale_t *) mem;
    if(!cself->closed) {
	j_client_close(cself->client);
    }
    pthread_mutex_destroy(&cself->lock);
    xfree(cself);
}

static int jscale_process(jack_nframes_t nframes, void *arg) {
    jscale_t *cself = (jscale_t *) arg;
    jack_default_audio_sample_t *in_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(cself->in_port, nframes);
    if(in_buffer == NULL) {
	return -1;
    }
    jack_default_audio_sample_t *out_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(cself->out_port, nframes);
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
	    out_buffer[i] = in_buffer[i] * cself->scale;
	}
    }
    r = j_mutex_unlock(&cself->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

static VALUE jscale_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname, rscale;
    char *name = "scale";
    jack_default_audio_sample_t scale = 1.0f;
    switch(rb_scan_args(argc, argv, "02", &rname, &rscale)) {
    case 2:
	scale = NUM2DBL(rscale);
    case 1:
	name = StringValueCStr(rname);
    }

    jscale_t *cself = ALLOC(jscale_t);
    jclient_init(name, 0, NULL, (jclient_t *) cself);

    cself->in_port = j_client_port_register(cself->client, "in", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(cself->in_port == NULL) {
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    cself->out_port = j_client_port_register(cself->client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(cself->out_port == NULL) {
	j_client_port_unregister(cself->client, cself->in_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    int r = j_mutex_init(&cself->lock);
    if(r != 0) {
	j_client_port_unregister(cself->client, cself->out_port);
	j_client_port_unregister(cself->client, cself->in_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(rb_eRuntimeError, "Could not create pthread mutex: %s", strerror(errno));
    }

    r = jack_set_process_callback(cself->client, jscale_process, cself);
    if(r != 0) {
	pthread_mutex_destroy(&cself->lock);
	j_client_port_unregister(cself->client, cself->out_port);
	j_client_port_unregister(cself->client, cself->in_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    cself->scale = scale;

    r = j_client_activate(cself->client);
    if(r != 0) {
	pthread_mutex_destroy(&cself->lock);
	j_client_port_unregister(cself->client, cself->out_port);
	j_client_port_unregister(cself->client, cself->in_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    VALUE self = Data_Wrap_Struct(klass, 0, jscale_free, cself);
    rb_iv_set(self, "@in", Data_Wrap_Struct(cJackPort, 0, fake_free, cself->in_port));
    rb_iv_set(self, "@out", Data_Wrap_Struct(cJackPort, 0, fake_free, cself->out_port));
    return self;
}

static VALUE jscale_in(VALUE self) {
    return rb_iv_get(self, "@in");
}

static VALUE jscale_set_in(VALUE self, VALUE port) {
    return jclient_connect(self, port, rb_iv_get(self, "@in"));
}

static VALUE jscale_out(VALUE self) {
    return rb_iv_get(self, "@out");
}

static VALUE jscale_get_scale(VALUE self) {
    jscale_t *cself;
    Data_Get_Struct(self, jscale_t, cself);
    jack_default_audio_sample_t value;
    int r = j_mutex_lock(&cself->lock);
    {
	if(r != 0) {
	    rb_raise(rb_eRuntimeError, "Could not acquire pthread mutex: %s", strerror(errno));
	}
	value = cself->scale;
    }
    r = j_mutex_unlock(&cself->lock);
    if(r != 0) {
	rb_raise(rb_eRuntimeError, "Could not release pthread mutex: %s", strerror(errno));
    }
    return DBL2NUM(value);
}

static VALUE jscale_set_scale(VALUE self, VALUE scale) {
    jscale_t *cself;
    Data_Get_Struct(self, jscale_t, cself);
    jack_default_audio_sample_t value = NUM2DBL(scale);
    int r = j_mutex_lock(&cself->lock);
    {
	if(r != 0) {
	    rb_raise(rb_eRuntimeError, "Could not acquire pthread mutex: %s", strerror(errno));
	}
	cself->scale = value;
    }
    r = j_mutex_unlock(&cself->lock);
    if(r != 0) {
	rb_raise(rb_eRuntimeError, "Could not release pthread mutex: %s", strerror(errno));
    }
    return scale;
}

void Init_scale() {
    cJScale = rb_define_class_under(mJack, "Scale", cJackClient);
    
    rb_define_singleton_method(cJScale, "new", jscale_new, -1);
    rb_define_method(cJScale, "in", jscale_in, 0);
    rb_define_method(cJScale, "in=", jscale_set_in, 1);
    rb_define_method(cJScale, "out", jscale_out, 0);
    rb_define_method(cJScale, "scale", jscale_get_scale, 0);
    rb_define_method(cJScale, "scale=", jscale_set_scale, 1);
}
