#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/portamento.h"
#include "cshellsynth/filter.h"
#include "atomic-float.h"

static int cs_porta_process(jack_nframes_t nframes, void *arg) {
    cs_porta_t *self = (cs_porta_t *) arg;
    float *in_buffer;
    float *lag_buffer;
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
    float lag = atomic_float_read(&self->lag);
    if(isnanf(lag)) {
	lag_buffer = (float *)jack_port_get_buffer(self->lag_port, nframes);
	if(lag_buffer == NULL) {
	    return -1;
	}
    }
    int i;
    for(i = 0; i < nframes; i++) {
	double c_in = (double) (isnanf(in) ? in_buffer[i] : in);
	if(c_in != self->target) {
	    self->target = c_in;
	    self->start = self->last;
	}
	if(self->last != self->target) {
	    self->last += (self->target - self->start) / ((double) (isnanf(lag) ? lag_buffer[i] : lag));
	    if(self->start <= self->target) {
		/* going up */
		if(self->last > self->target) {
		    self->last = self->target;
		}
	    } else {
		/* going down */
		if(self->last < self->target) {
		    self->last = self->target;
		}
	    }
	}
	out_buffer[i] = self->last;
    }
    return 0;
}

void cs_porta_set_lag(cs_porta_t *self, float lag) {
    atomic_float_set(&self->lag, lag * jack_get_sample_rate(self->client));
}

int cs_porta_init(cs_porta_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_filter_init((cs_filter_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->lag_port = jack_port_register(self->client, "lag", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->lag_port == NULL) {
	cs_filter_destroy((cs_filter_t *) self);
	return -1;
    };

    atomic_float_set(&self->lag, NAN);
    self->start = 0.0;
    self->target = 0.0;
    self->last = 0.0;

    r = jack_set_process_callback(self->client, cs_porta_process, self);
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