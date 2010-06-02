#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/modulator.h"
#include "cshellsynth/mixer.h"
#include "cshellsynth/jclient.h"

static int cs_modu_process(jack_nframes_t nframes, void *arg) {
    cs_modu_t *self = (cs_modu_t *) arg;
    jack_default_audio_sample_t *in1_buffer;
    jack_default_audio_sample_t *in2_buffer;
    jack_default_audio_sample_t *out_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	if(isnanf(self->in1)) {
	    in1_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(self->in1_port, nframes);
	    if(in1_buffer == NULL) {
		return -1;
	    }
	}
	if(isnanf(self->in2)) {
	    in2_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(self->in2_port, nframes);
	    if(in2_buffer == NULL) {
		return -1;
	    }
	}
	int i;
	for(i = 0; i < nframes; i++) {
	    out_buffer[i] = (isnanf(self->in1) ? in1_buffer[i] : self->in1) * (isnanf(self->in2) ? in2_buffer[i] : self->in2);
	}
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

int cs_modu_init(cs_modu_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_mix_subclass_init((cs_mix_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    r = jack_set_process_callback(self->client, cs_modu_process, self);
    if(r != 0) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return r;
    }

    r = jack_activate(self->client);
    if(r != 0) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return r;
    }

    return 0;
}
