#include <jack/jack.h>
#include <math.h>
#include "instrument.h"
#include "controller.h"

static int cs_inst_process(jack_nframes_t nframes, void *arg) {
    cs_inst_t *self = (cs_inst_t *) arg;
    jack_default_audio_sample_t *out_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    jack_default_audio_sample_t *ctl_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(self->ctl_port, nframes);
    if(ctl_buffer == NULL) {
	return -1;
    }

    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	int i;
	for(i = 0; i < nframes; i++) {
	    if((self->value != self->last_value)
	       && !(isnanf(self->value) && isnanf(self->last_value))) {
		if(isnanf(self->value)) {
		    ctl_buffer[i] = -1.0f;
		} else {
		    ctl_buffer[i] = 1.0f;
		}
	    } else {
		ctl_buffer[i] = 0.0f;
	    }
	    self->last_value = self->value;
	    out_buffer[i] = self->value;
	}
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

int cs_inst_init(cs_inst_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_ctlr_init((cs_ctlr_t *) self, client_name, flags, server_name);
    if(r != 0) {
	cs_ctlr_destroy((cs_ctlr_t *) self);
	return r;
    }

    self->value = NAN;
    self->last_value = NAN;

    r = jack_set_process_callback(self->client, cs_inst_process, self);
    if(r != 0) {
	cs_ctlr_destroy((cs_ctlr_t *) self);
	return r;
    }

    r = jack_activate(self->client);
    if(r != 0) {
	cs_ctlr_destroy((cs_ctlr_t *) self);
	return r;
    }

    return 0;
}

int cs_inst_play(cs_inst_t *self, jack_default_audio_sample_t value) {
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	self->value = value;
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }

    return 0;
}

int cs_inst_stop(cs_inst_t *self) {
    return cs_inst_play(self, NAN);
}