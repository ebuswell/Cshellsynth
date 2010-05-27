#include <ruby.h>
#include <jack/jack.h>
#include <math.h>
#include "jackruby.h"
#include "synths.h"
#include <errno.h>

static VALUE cJSine;

typedef struct jsine_struct {
    jack_client_t *client;
    int closed;
    jack_port_t *freq_port;
    jack_port_t *out_port;
    jack_default_audio_sample_t f;
    jack_default_audio_sample_t period;
    jack_default_audio_sample_t wt;
    jack_default_audio_sample_t offset;
    jack_nframes_t sample_rate;
} jsine_t;

static int jsine_process(jack_nframes_t nframes, void *arg) {
    jsine_t *cself = (jsine_t *) arg;
    jack_default_audio_sample_t *freq_buffer = (jack_default_audio_sample_t *) jack_port_get_buffer(cself->freq_port, nframes);
    if(freq_buffer == NULL) {
	return -1;
    }
    jack_default_audio_sample_t *out_buffer = (jack_default_audio_sample_t *) jack_port_get_buffer(cself->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    int i;
    for(i = 0; i < nframes; i++) {
	jack_default_audio_sample_t f = freq_buffer[i];
	if(f == 0.0f || f == NAN) {
	    cself->offset = 0;
	    out_buffer[i] = 0.0f;
	    cself->f = f;
	} else {
	    if(f != cself->f) {
		cself->wt = (2 * M_PI * f) / ((jack_default_audio_sample_t) cself->sample_rate);
		cself->period = ((jack_default_audio_sample_t) cself->sample_rate) / f;
		cself->f = f;
	    }
	    cself->offset += 1.0f;
	    if(cself->offset > cself->period) {
		cself->offset -= cself->period;
	    }
	    out_buffer[i] = sinf(cself->wt * ((jack_default_audio_sample_t) cself->offset));
	}
    }
    return 0;
}

static VALUE jsine_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "sine";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }

    jsine_t *cself = ALLOC(jsine_t);
    jclient_init(name, 0, NULL, (jclient_t *) cself);

    cself->freq_port = j_client_port_register(cself->client, "freq", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(cself->freq_port == NULL) {
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    cself->out_port = j_client_port_register(cself->client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(cself->out_port == NULL) {
	j_client_port_unregister(cself->client, cself->freq_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    int r = jack_set_process_callback(cself->client, jsine_process, cself);
    if(r != 0) {
	j_client_port_unregister(cself->client, cself->out_port);
	j_client_port_unregister(cself->client, cself->freq_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Could not set process callback");
    }

    cself->offset = 0.0f;
    cself->period = 0.0f;
    cself->f = 0.0f;
    cself->wt = 0.0f;
    cself->sample_rate = jack_get_sample_rate(cself->client);

    r = j_client_activate(cself->client);
    if(r != 0) {
	j_client_port_unregister(cself->client, cself->out_port);
	j_client_port_unregister(cself->client, cself->freq_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    VALUE self = Data_Wrap_Struct(klass, 0, jclient_free, cself);
    rb_iv_set(self, "@freq", Data_Wrap_Struct(cJackPort, 0, fake_free, cself->freq_port));
    rb_iv_set(self, "@out", Data_Wrap_Struct(cJackPort, 0, fake_free, cself->out_port));
    return self;
}

static VALUE jsine_freq(VALUE self) {
    return rb_iv_get(self, "@freq");
}

static VALUE jsine_set_freq(VALUE self, VALUE port) {
    return jclient_connect(self, port, rb_iv_get(self, "@freq"));
}

static VALUE jsine_out(VALUE self) {
    return rb_iv_get(self, "@out");
}

void Init_sine() {
    cJSine = rb_define_class_under(mJSynths, "Sine", cJackClient);

    rb_define_singleton_method(cJSine, "new", jsine_new, -1);
    rb_define_method(cJSine, "freq", jsine_freq, 0);
    rb_define_method(cJSine, "freq=", jsine_set_freq, 1);
    rb_define_method(cJSine, "out", jsine_out, 0);
}