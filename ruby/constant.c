#include <ruby.h>
#include <jack/jack.h>
#include "jackruby.h"
#include <math.h>
#include <errno.h>

static VALUE cJConstant;

typedef struct jconst_struct {
    jack_client_t *client;
    int closed;
    pthread_mutex_t lock;
    jack_port_t *out_port;
    jack_default_audio_sample_t value;
} jconst_t;

static void jconst_free(void *mem) {
    jconst_t *cself = (jconst_t *) mem;
    if(!cself->closed) {
	j_client_close(cself->client);
    }
    pthread_mutex_destroy(&cself->lock);
    xfree(cself);
}

static int jconst_process(jack_nframes_t nframes, void *arg) {
    jconst_t *cself = (jconst_t *) arg;
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
	for(i = 0; i < nframes; i++)
	    out_buffer[i] = cself->value;
    }
    r = j_mutex_unlock(&cself->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

static VALUE jconst_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname, rvalue;
    char *name = "timer";
    jack_default_audio_sample_t value = NAN;
    switch(rb_scan_args(argc, argv, "03", &rname, &rvalue)) {
    case 2:
	value = NUM2DBL(rvalue);
    case 1:
	name = StringValueCStr(rname);
    }

    jconst_t *cself = ALLOC(jconst_t);
    jclient_init(name, 0, NULL, (jclient_t *) cself);

    cself->out_port = j_client_port_register(cself->client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(cself->out_port == NULL) {
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    int r = j_mutex_init(&cself->lock);
    if(r != 0) {
	j_client_port_unregister(cself->client, cself->out_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(rb_eRuntimeError, "Could not create pthread mutex: %s", strerror(errno));
    }

    r = jack_set_process_callback(cself->client, jconst_process, cself);
    if(r != 0) {
	pthread_mutex_destroy(&cself->lock);
	j_client_port_unregister(cself->client, cself->out_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Could not set process callback");
    }

    cself->value = value;

    r = j_client_activate(cself->client);
    if(r != 0) {
	pthread_mutex_destroy(&cself->lock);
	j_client_port_unregister(cself->client, cself->out_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Could not set process callback");
    }

    VALUE self = Data_Wrap_Struct(klass, 0, jconst_free, cself);
    rb_iv_set(self, "@out", Data_Wrap_Struct(cJackPort, 0, fake_free, cself->out_port));
    return self;
}

static VALUE jconst_out(VALUE self) {
    return rb_iv_get(self, "@out");
}

static VALUE jconst_set_value(VALUE self, VALUE rvalue) {
    jconst_t *cself;
    Data_Get_Struct(self, jconst_t, cself);
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

void Init_constant() {
    cJConstant = rb_define_class_under(mJack, "Constant", cJackClient);
    
    rb_define_singleton_method(cJConstant, "new", jconst_new, -1);
    rb_define_method(cJConstant, "out", jconst_out, 0);
    rb_define_method(cJConstant, "value=", jconst_set_value, 1);
}
