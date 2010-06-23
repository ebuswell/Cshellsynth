#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/lowpass.h"
#include "cshellsynth/filter.h"
#include "atomic-float.h"

static int cs_lowpass_process(jack_nframes_t nframes, void *arg) {
    cs_lowpass_t *self = (cs_lowpass_t *) arg;
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
    double sample_rate = (double) jack_get_sample_rate(self->client);
    int i;
    for(i = 0; i < nframes; i++) {
	double x = exp(-2.0 * M_PI * ((double) (isnanf(freq) ? freq_buffer[i] : freq)) / sample_rate);
	self->last_out = (1.0 - x) * ((double) (isnanf(in) ? in_buffer[i] : in))
	    + x * self->last_out;
	out_buffer[i] = (float) self->last_out;
    }
    return 0;
}

void cs_lowpass_set_freq(cs_lowpass_t *self, float freq) {
    atomic_float_set(&self->freq, freq);
}

int cs_lowpass_init(cs_lowpass_t *self, const char *client_name, jack_options_t flags, char *server_name) {
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

    r = jack_set_process_callback(self->client, cs_lowpass_process, self);
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
