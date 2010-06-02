#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/square.h"
#include "cshellsynth/synth.h"
#include "cshellsynth/jclient.h"

static int cs_square_process(jack_nframes_t nframes, void *arg) {
    cs_square_t *self = (cs_square_t *) arg;
    jack_default_audio_sample_t *freq_buffer;
    jack_default_audio_sample_t *out_buffer = (jack_default_audio_sample_t *) jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    double sample_rate = (double) jack_get_sample_rate(self->client);
    double sample_period = 1.0 / sample_rate;
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	if(isnanf(self->freq)) {
	    freq_buffer = (jack_default_audio_sample_t *) jack_port_get_buffer(self->freq_port, nframes);
	    if(freq_buffer == NULL) {
		return -1;
	    }
	}
	int i;
	for(i = 0; i < nframes; i++) {
	    jack_default_audio_sample_t f = isnanf(self->freq) ? freq_buffer[i] : self->freq;
	    if(f == 0.0f || isnanf(f)) {
		self->offset = 0.0;
		out_buffer[i] = 0.0f;
	    } else {
		double period = sample_rate / f;
		while(self->offset >= period) {
		    self->offset -= period;
		}
		if(self->offset >= (period / 2.0)) {
		    if(self->offset + sample_period > period) {
			// percentage of sample before transition
			double diff = (period - self->offset) / sample_period;
			// average value is (1.0 * diff) + (-1.0 * (1 - diff))
			// diff - 1 + diff
			// 2*diff - 1
			out_buffer[i] = (2.0 * diff) - 1.0f;
		    } else {
			out_buffer[i] = 1.0f;
		    }
		} else {
		    if(self->offset + sample_period > (period / 2.0)) {
			// percentage of sample before transition
			jack_default_audio_sample_t diff = ((period / 2.0) - self->offset) / sample_period;
			// average value is (-1.0 * diff) + (1.0 * (1 - diff))
			// -diff + 1 - diff
			// 1 - 2*diff
			out_buffer[i] = 1.0f - (2.0 * diff);
		    } else {
			out_buffer[i] = -1.0f;
		    }
		}
		self->offset += 1.0;
	    }
	}
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

int cs_square_init(cs_square_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_synth_init((cs_synth_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }
    r = jack_set_process_callback(self->client, cs_square_process, self);
    if(r != 0) {
	cs_synth_destroy((cs_synth_t *) self);
	return r;
    }
    self->offset = 0.0;
    r = jack_activate(self->client);
    if(r != 0) {
	cs_synth_destroy((cs_synth_t *) self);
	return r;
    }
    return 0;
}
