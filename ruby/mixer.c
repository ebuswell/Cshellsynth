#include <ruby.h>
#include <jack/jack.h>
#include "jackruby.h"
#include <errno.h>

static VALUE cJMixer;

typedef struct jmix_struct {
    jack_client_t *client;
    int closed;
    jack_port_t **in_ports;
    size_t in_ports_size;
    jack_port_t *out_port;
} jmix_t;

static int jmix_process(jack_nframes_t nframes, void *arg) {
    jmix_t *jmix = (jmix_t *) arg;
    jack_default_audio_sample_t *out_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(jmix->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    size_t i = 0;
    jack_nframes_t j;
    jack_default_audio_sample_t *in_buffer;
    if(jmix->in_ports_size > 0) {
	in_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(jmix->in_ports[0], nframes);
	if(in_buffer == NULL) {
	    return -1;
	}
	memcpy(out_buffer, in_buffer, nframes * sizeof(jack_default_audio_sample_t));
	i++;
    }
    for(;i < jmix->in_ports_size; i++) {
	in_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(jmix->in_ports[i], nframes);
	if(in_buffer == NULL) {
	    return -1;
	}
	for(j = 0; j < nframes; j++) {
	    out_buffer[j] += in_buffer[j];
	}
    }
    return 0;
}

void jmix_free(void *mem) {
    jmix_t *cself = mem;
    if(!cself->closed) {
	j_client_close(cself->client);
    }
    xfree(cself->in_ports);
    xfree(cself);
}

static VALUE jmix_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname, rnports;
    char *name = "mixer";
    size_t nports = 4;
    switch(rb_scan_args(argc, argv, "02", &rname, &rnports)) {
    case 2:
	nports = NUM2SIZET(rnports);
	if((nports > 999) || (nports < 1)) {
	    rb_raise(rb_eArgError, "ports must be between 1 and 999");
	}
    case 1:
	name = StringValueCStr(rname);
    }

    jmix_t *cself = ALLOC(jmix_t);
    jclient_init(name, 0, NULL, (jclient_t *) cself);

    cself->in_ports = ALLOC_N(jack_port_t *, nports);
    char *reg_name = ALLOCA_N(char, 6);
    size_t i;
    for(i = 0; i < nports; i++) {
	snprintf(reg_name, 6, "in%ld", i + 1);
	cself->in_ports[i] = j_client_port_register(cself->client, reg_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
	if(cself->in_ports[i] == NULL) {
	    for(; i >= 0; i--) {
		j_client_port_unregister(cself->client, cself->in_ports[i]);
	    }
	    j_client_close(cself->client);
	    xfree(cself->in_ports);
	    xfree(cself);
	    rb_raise(eJackFailure, "Overall operation failed");
	}
    }

    cself->out_port = j_client_port_register(cself->client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(cself->out_port == NULL) {
	for(i = nports; i >= 0; i--) {
	    j_client_port_unregister(cself->client, cself->in_ports[i]);
	}
	j_client_close(cself->client);
	xfree(cself->in_ports);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    int r = jack_set_process_callback(cself->client, jmix_process, cself);
    if(r != 0) {
	j_client_port_unregister(cself->client, cself->out_port);
	for(i = nports; i >= 0; i--) {
	    j_client_port_unregister(cself->client, cself->in_ports[i]);
	}
	j_client_close(cself->client);
	xfree(cself->in_ports);
	xfree(cself);
	rb_raise(eJackFailure, "Could not set process callback");
    }

    cself->in_ports_size = nports;

    r = j_client_activate(cself->client);
    if(r != 0) {
	j_client_port_unregister(cself->client, cself->out_port);
	for(i = nports; i >= 0; i--) {
	    j_client_port_unregister(cself->client, cself->in_ports[i]);
	}
	j_client_close(cself->client);
	xfree(cself->in_ports);
	xfree(cself);
	rb_raise(eJackFailure, "Could not set process callback");
    }

    VALUE self = Data_Wrap_Struct(klass, 0, jmix_free, cself);
    VALUE rin_ports = rb_ary_new2(nports);
    for(i = 0; i < nports; i++) {
	rb_ary_push(rin_ports, Data_Wrap_Struct(cJackPort, 0, fake_free, cself->in_ports[i]));
    }
    rb_iv_set(self, "@in", rin_ports);
    rb_iv_set(self, "@out", Data_Wrap_Struct(cJackPort, 0, fake_free, cself->out_port));
    return self;
}

static VALUE jmix_in(VALUE self) {
    return rb_iv_get(self, "@in");
}

static VALUE jmix_set_in(VALUE self, VALUE port) {
    jmix_t *cself;
    Data_Get_Struct(self, jmix_t, cself);
    int i;
    for(i = 0; i < cself->in_ports_size; i++) {
	if(!jack_port_connected(cself->in_ports[i])) {
	    return jclient_connect(self, port, rb_ary_entry(rb_iv_get(self, "@in"), i));
	}
    }
    return rb_iv_get(self, "@in");
}

static VALUE jmix_out(VALUE self) {
    return rb_iv_get(self, "@out");
}

void Init_mixer() {
    cJMixer = rb_define_class_under(mJack, "Mixer", cJackClient);
    
    rb_define_singleton_method(cJMixer, "new", jmix_new, -1);
    rb_define_method(cJMixer, "in", jmix_in, 0);
    rb_define_method(cJMixer, "in=", jmix_set_in, 1);
    rb_define_method(cJMixer, "out", jmix_out, 0);
}
