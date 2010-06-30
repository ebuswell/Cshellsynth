#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/highpass.h"
#include "cshellsynth/filter.h"
#include "atomic-float.h"

static int cs_highpass_process(jack_nframes_t nframes, void *arg) {
    cs_highpass_t *self = (cs_highpass_t *) arg;
    float *in_buffer;
    float *freq_buffer;
    float *out_buffer = (float *)jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    float in = atomic_float_read(&self->in);
    if(isnanf(in)) {
	in_buffer = (float *)jack_port_get_buffer(self->in_port, nframes);
	if(in_buffer == NULL) {
	    return -1;
	}
    }
    float freq = atomic_float_read(&self->freq);
    if(isnanf(freq)) {
	freq_buffer = (float *)jack_port_get_buffer(self->freq_port, nframes);
	if(freq_buffer == NULL) {
	    return -1;
	}
    }
    int i;
    for(i = 0; i < nframes; i++) {
	double a = 1.0/((2.0 * M_PI * ((double) (isnanf(freq) ? freq_buffer[i] : freq))) + 1.0);
	float c_in = (isnanf(in) ? in_buffer[i] : in);
	self->last_out = a * (((double) (c_in  - self->last_in))
			      + self->last_out);
	self->last_in = c_in;
	out_buffer[i] = (float) self->last_out;
    }
    return 0;
}

void cs_highpass_set_freq(cs_highpass_t *self, float freq) {
    if(freq > 1.0) {
	freq = freq / jack_get_sample_rate(self->client);
    }
    atomic_float_set(&self->freq, freq);
}

int cs_highpass_init(cs_highpass_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_filter_init((cs_filter_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->freq_port = jack_port_register(self->client, "freq", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->freq_port == NULL) {
	cs_filter_destroy((cs_filter_t *) self);
	return -1;
    };

    atomic_float_set(&self->freq, NAN);
    self->last_out = 0.0;
    self->last_in = 0.0f;

    r = jack_set_process_callback(self->client, cs_highpass_process, self);
    if(r != 0) {
	cs_filter_destroy((cs_filter_t *) self);
	return r;
    }

    r = jack_activate(self->client);
    if(r != 0) {
	cs_filter_destroy((cs_filter_t *) self);
	return r;
    }

    return 0;
}
