#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/lin2exp.h"
#include "cshellsynth/filter.h"

static int cs_lin2exp_process(jack_nframes_t nframes, void *arg) {
    cs_lin2exp_t *self = (cs_lin2exp_t *) arg;
    jack_default_audio_sample_t *in_buffer;
    jack_default_audio_sample_t *out_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	if(isnanf(self->in)) {
	    in_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(self->in_port, nframes);
	    if(in_buffer == NULL) {
		pthread_mutex_unlock(&self->lock);
		return -1;
	    }
	}
	int i;
	for(i = 0; i < nframes; i++) {
	    out_buffer[i] = (powf(self->zero, 1.0f - (isnanf(self->in) ? in_buffer[i] : self->in)) - self->zero) / (1.0f - self->zero);
	}
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

int cs_lin2exp_set_zero(cs_lin2exp_t *self, jack_default_audio_sample_t zero) {
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	self->zero = zero;
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

int cs_lin2exp_init(cs_lin2exp_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_filter_init((cs_filter_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->zero = 0.0625;

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
