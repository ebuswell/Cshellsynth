#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/lowpass.h"
#include "cshellsynth/filter.h"

static int cs_lowpass_process(jack_nframes_t nframes, void *arg) {
    cs_lowpass_t *self = (cs_lowpass_t *) arg;
    jack_default_audio_sample_t *in_buffer;
    jack_default_audio_sample_t *freq_buffer;
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
	if(isnanf(self->freq)) {
	    freq_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(self->freq_port, nframes);
	    if(freq_buffer == NULL) {
		pthread_mutex_unlock(&self->lock);
		return -1;
	    }
	}
	float sample_rate_adjust = (float) (-M_2_PI / (float) jack_get_sample_rate(self->client));
	int i;
	for(i = 0; i < nframes; i++) {
	    float in = (isnanf(self->in) ? in_buffer[i] : self->in);
	    if(isnanf(in)) in = 0.0f;
	    float x = expf(sample_rate_adjust*(isnanf(self->freq) ? freq_buffer[i] : self->freq));
	    self->last_out = (1.0f - x)*in + x*self->last_out;
	    out_buffer[i] = self->last_out;
	}
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

int cs_lowpass_set_freq(cs_lowpass_t *self, jack_default_audio_sample_t freq) {
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	self->freq = freq;
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
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

    self->freq = NAN;
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
