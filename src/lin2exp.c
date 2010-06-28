#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/lin2exp.h"
#include "cshellsynth/filter.h"
#include "atomic-float.h"

static int cs_lin2exp_process(jack_nframes_t nframes, void *arg) {
    cs_lin2exp_t *self = (cs_lin2exp_t *) arg;
    float *in_buffer;
    float *zero_buffer;
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
    float zero = atomic_float_read(&self->zero);
    if(isnanf(zero)) {
	zero_buffer = (float *)jack_port_get_buffer(self->zero_port, nframes);
	if(zero_buffer == NULL) {
	    return -1;
	}
    }
    int i;
    for(i = 0; i < nframes; i++) {
	float x = isnanf(in) ? in_buffer[i] : in;
	/* out_buffer[i] = (powf(zero, 1.0f - x) - zero) / (1.0f - zero); */
	out_buffer[i] = zero*exp(x);
    }
    return 0;
}

void cs_lin2exp_set_zero(cs_lin2exp_t *self, float zero) {
    atomic_float_set(&self->zero, zero);
}

int cs_lin2exp_init(cs_lin2exp_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_filter_init((cs_filter_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }
    self->zero_port = jack_port_register(self->client, "zero", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->zero_port == NULL) {
	cs_filter_destroy((cs_filter_t *) self);
	return -1;
    }

    atomic_float_set(&self->zero, NAN);

    r = jack_set_process_callback(self->client, cs_lin2exp_process, self);
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
