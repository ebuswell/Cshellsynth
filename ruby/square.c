#include <ruby.h>
#include <jack/jack.h>
#include <math.h>
#include "jackruby.h"
#include "synths.h"
#include <errno.h>

static VALUE cJSquare;

typedef struct jsquare_struct {
    jack_client_t *client;
    int closed;
    jack_port_t *freq_port;
    jack_port_t *out_port;
    jack_default_audio_sample_t freq;
    jack_default_audio_sample_t period;
    jack_default_audio_sample_t offset;
    jack_default_audio_sample_t sample_period;
    jack_nframes_t sample_rate;
} jsquare_t;

static int jsquare_process(jack_nframes_t nframes, void *arg) {
    jsquare_t *cself = (jsquare_t *) arg;
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
	jack_default_audio_sample_t freq = freq_buffer[i];
	if(freq == 0.0f || freq == NAN) {
	    cself->offset = 0.0f;
	    out_buffer[i] = 0.0f;
	    cself->freq = freq;
	} else {
	    if(freq != cself->freq) {
		cself->period = ((jack_default_audio_sample_t) cself->sample_rate) / freq;
		cself->freq = freq;
	    }
	    cself->offset += 1;
	    if(cself->offset >= cself->period) {
		cself->offset -= cself->period;
	    }
	    if(cself->offset >= (cself->period / 2)) {
		if(cself->offset + cself->sample_period > cself->period) {
		    // percentage of sample before transition
		    jack_default_audio_sample_t diff = (cself->period - cself->offset) / cself->sample_period;
		    // average value is (1.0 * diff) + (-1.0 * (1 - diff))
		    // diff - 1 + diff
		    // 2*diff - 1
		    out_buffer[i] = 2.0f * diff - 1.0f;
		} else {
		    out_buffer[i] = 1.0f;
		}
	    } else {
		if(cself->offset + cself->sample_period > (cself->period / 2)) {
		    // percentage of sample before transition
		    jack_default_audio_sample_t diff = ((cself->period / 2) - cself->offset) / cself->sample_period;
		    // average value is (-1.0 * diff) + (1.0 * (1 - diff))
		    // -diff + 1 - diff
		    // 1 - 2*diff
		    out_buffer[i] = 1.0f - 2.0f * diff;
		} else {
		    out_buffer[i] = -1.0f;
		}
	    }
	}
    }
    return 0;
}

static VALUE jsquare_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "square";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }

    jsquare_t *cself = ALLOC(jsquare_t);
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

    int r = jack_set_process_callback(cself->client, jsquare_process, cself);
    if(r != 0) {
	j_client_port_unregister(cself->client, cself->out_port);
	j_client_port_unregister(cself->client, cself->freq_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Could not set process callback");
    }

    cself->offset = 0.0f;
    cself->freq = 0.0f;
    cself->period = 0.0f;
    cself->sample_rate = jack_get_sample_rate(cself->client);
    cself->sample_period = 1.0f / ((jack_default_audio_sample_t) cself->sample_rate);

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

static VALUE jsquare_freq(VALUE self) {
    return rb_iv_get(self, "@freq");
}

static VALUE jsquare_out(VALUE self) {
    return rb_iv_get(self, "@out");
}

static VALUE jsquare_set_freq(VALUE self, VALUE port) {
    return jclient_connect(self, port, rb_iv_get(self, "@freq"));
}

void Init_square() {
    cJSquare = rb_define_class_under(mJSynths, "Square", cJackClient);
    
    rb_define_singleton_method(cJSquare, "new", jsquare_new, -1);
    rb_define_method(cJSquare, "freq", jsquare_freq, 0);
    rb_define_method(cJSquare, "freq=", jsquare_set_freq, 1);
    rb_define_method(cJSquare, "out", jsquare_out, 0);
}
