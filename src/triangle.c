#include <jack/jack.h>
#include <math.h>
#include "triangle.h"
#include "synth.h"
#include "jclient.h"

static int cs_triangle_process(jack_nframes_t nframes, void *arg) {
    cs_triangle_t *self = (cs_triangle_t *) arg;
    jack_default_audio_sample_t *freq_buffer;
    jack_default_audio_sample_t *out_buffer = (jack_default_audio_sample_t *) jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    double sample_rate = (double) jack_get_sample_rate(self->client);
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
		// /\
		//   \/
		// 0123
		//  /\
		// /  \
		// 3012
		double period = sample_rate / f;
		while(self->offset >= period) {
		    self->offset -= period;
		}
		double offset = self->offset + (period / 4);
		if(offset >= period) {
		    offset -= period;
		}
		jack_default_audio_sample_t a = 4.0 * (offset / period);
		if(a > 2.0f) {
		    a = 4.0f - a;
		}
		out_buffer[i] = a - 1.0f;
		self->offset += 1.0f;
	    }
	}
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

int cs_triangle_init(cs_triangle_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_synth_init((cs_synth_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }
    r = jack_set_process_callback(self->client, cs_triangle_process, self);
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
