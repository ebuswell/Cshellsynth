#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/mixer.h"
#include "cshellsynth/jclient.h"

int cs_mix_set_in1(cs_mix_t *self, jack_default_audio_sample_t in1) {
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	self->in1 = in1;
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
}

int cs_mix_set_in2(cs_mix_t *self, jack_default_audio_sample_t in2) {
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	self->in2 = in2;
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
}

static int cs_mix_process(jack_nframes_t nframes, void *arg) {
    cs_mix_t *self = (cs_mix_t *) arg;
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
	    out_buffer[i] = (isnanf(self->in1) ? in1_buffer[i] : self->in1) + (isnanf(self->in2) ? in2_buffer[i] : self->in2);
	}
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

int cs_mix_subclass_init(cs_mix_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = jclient_locking_init((jclient_locking_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->in1_port = jack_port_register(self->client, "in1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->in1_port == NULL) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return -1;
    }

    self->in2_port = jack_port_register(self->client, "in2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->in2_port == NULL) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return -1;
    }

    self->out_port = jack_port_register(self->client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(self->out_port == NULL) {
	jclient_locking_destroy((jclient_locking_t *) self);
	return -1;
    }

    self->in1 = NAN;
    self->in2 = NAN;
    
    return 0;
}

int cs_mix_init(cs_mix_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_mix_subclass_init(self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    r = jack_set_process_callback(self->client, cs_mix_process, self);
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
