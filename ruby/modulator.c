#include <ruby.h>
#include <jack/jack.h>
#include "jackruby.h"
#include <errno.h>

static VALUE cJModulator;

typedef struct jmodu_struct {
    jack_client_t *client;
    int closed;
    jack_port_t *in1_port;
    jack_port_t *in2_port;
    jack_port_t *out_port;
} jmodu_t;

static int jmodu_process(jack_nframes_t nframes, void *arg) {
    jmodu_t *jmodu = (jmodu_t *) arg;
    jack_default_audio_sample_t *in1_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(jmodu->in1_port, nframes);
    if(in1_buffer == NULL) {
	return -1;
    }
    jack_default_audio_sample_t *in2_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(jmodu->in2_port, nframes);
    if(in2_buffer == NULL) {
	return -1;
    }
    jack_default_audio_sample_t *out_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(jmodu->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    int i;
    for(i = 0; i < nframes; i++) {
	out_buffer[i] = in1_buffer[i] * in2_buffer[i];
    }
    return 0;
}

static VALUE jmodu_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "modulator";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }

    jmodu_t *cself = ALLOC(jmodu_t);
    jclient_init(name, 0, NULL, (jclient_t *) cself);

    cself->in1_port = j_client_port_register(cself->client, "in1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(cself->in1_port == NULL) {
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    cself->in2_port = j_client_port_register(cself->client, "in2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(cself->in2_port == NULL) {
	j_client_port_unregister(cself->client, cself->in1_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    cself->out_port = j_client_port_register(cself->client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(cself->out_port == NULL) {
	j_client_port_unregister(cself->client, cself->in2_port);
	j_client_port_unregister(cself->client, cself->in1_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    int r = jack_set_process_callback(cself->client, jmodu_process, cself);
    if(r != 0) {
	j_client_port_unregister(cself->client, cself->out_port);
	j_client_port_unregister(cself->client, cself->in1_port);
	j_client_port_unregister(cself->client, cself->in2_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    r = j_client_activate(cself->client);
    if(r != 0) {
	j_client_port_unregister(cself->client, cself->out_port);
	j_client_port_unregister(cself->client, cself->in1_port);
	j_client_port_unregister(cself->client, cself->in2_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    VALUE self = Data_Wrap_Struct(klass, 0, jclient_free, cself);
    rb_iv_set(self, "@in1", Data_Wrap_Struct(cJackPort, 0, fake_free, cself->in1_port));
    rb_iv_set(self, "@in2", Data_Wrap_Struct(cJackPort, 0, fake_free, cself->in2_port));
    rb_iv_set(self, "@out", Data_Wrap_Struct(cJackPort, 0, fake_free, cself->out_port));
    return self;
}

static VALUE jmodu_in1(VALUE self) {
    return rb_iv_get(self, "@in1");
}

static VALUE jmodu_in2(VALUE self) {
    return rb_iv_get(self, "@in2");
}

static VALUE jmodu_set_in1(VALUE self, VALUE port) {
    return jclient_connect(self, port, rb_iv_get(self, "@in1"));
}

static VALUE jmodu_set_in2(VALUE self, VALUE port) {
    return jclient_connect(self, port, rb_iv_get(self, "@in2"));
}

static VALUE jmodu_out(VALUE self) {
    return rb_iv_get(self, "@out");
}

void Init_modulator() {
    cJModulator = rb_define_class_under(mJack, "Modulator", cJackClient);
    
    rb_define_singleton_method(cJModulator, "new", jmodu_new, -1);
    rb_define_method(cJModulator, "in1", jmodu_in1, 0);
    rb_define_method(cJModulator, "in2", jmodu_in2, 0);
    rb_define_method(cJModulator, "in1=", jmodu_set_in1, 1);
    rb_define_method(cJModulator, "in2=", jmodu_set_in2, 1);
    rb_define_method(cJModulator, "out", jmodu_out, 0);
}
