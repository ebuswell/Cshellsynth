#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/distortion.h"
#include "cshellsynth/filter.h"
#include "atomic-float.h"

static int cs_distort_process(jack_nframes_t nframes, void *arg) {
    cs_distort_t *self = (cs_distort_t *) arg;
    float *in_buffer;
    float *gain_buffer;
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
    float gain = atomic_float_read(&self->gain);
    if(isnanf(gain)) {
	gain_buffer = (float *)jack_port_get_buffer(self->gain_port, nframes);
	if(gain_buffer == NULL) {
	    return -1;
	}
    }
    float sharpness = atomic_float_read(&self->sharpness);
    double factor = 1.0/(log(exp((double) sharpness) + 1.0));
    int i;
    for(i = 0; i < nframes; i++) {
	float c_in = isnanf(in) ? in_buffer[i] : in;
	float c_gain = isnanf(gain) ? gain_buffer[i] : gain;
	if(c_in >= 0.0f) {
	    out_buffer[i] = (float) (
		1.0 - factor
		* log(exp(-((double) sharpness) * (((double) c_in)*((double) c_gain) - 1.0)) + 1.0));
	} else {
	    out_buffer[i] = (float) (
		factor
		* log(exp(((double) sharpness) * (((double) c_in)*((double) c_gain) + 1.0)) + 1.0)
		- 1.0);
	}
    }
    return 0;
}

void cs_distort_set_gain(cs_distort_t *self, float gain) {
    atomic_float_set(&self->gain, gain);
}

void cs_distort_set_sharpness(cs_distort_t *self, float sharpness) {
    atomic_float_set(&self->sharpness, sharpness);
}

int cs_distort_init(cs_distort_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_filter_init((cs_filter_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->gain_port = jack_port_register(self->client, "gain", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->gain_port == NULL) {
	cs_filter_destroy((cs_filter_t *) self);
	return -1;
    };

    atomic_float_set(&self->gain, 1.0f);
    atomic_float_set(&self->sharpness, 2.0f);

    r = jack_set_process_callback(self->client, cs_distort_process, self);
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
