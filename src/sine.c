#include <jack/jack.h>
#include <math.h>
#include <stdbool.h>
#include "cshellsynth/sine.h"
#include "cshellsynth/synth.h"
#include "cshellsynth/jclient.h"
#include "atomic-float.h"

static int cs_sine_process(jack_nframes_t nframes, void *arg) {
    cs_sine_t *self = (cs_sine_t *) arg;
    float *freq_buffer;
    float *out_buffer = (float *) jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    double sample_rate = (double) jack_get_sample_rate(self->client);
    float freq = atomic_float_read(&self->freq);
    if(isnanf(freq)) {
	freq_buffer = (float *) jack_port_get_buffer(self->freq_port, nframes);
	if(freq_buffer == NULL) {
	    return -1;
	}
    }
    int i;
    for(i = 0; i < nframes; i++) {
	double f = (double) (isnanf(freq) ? freq_buffer[i] : freq);
	if(f == 0.0 || isnan(f)) {
	    self->f_t_1 = 0.0;
	    self->f_t_2 = 0.0;
	    out_buffer[i] = 0.0f;
	} else {
	    double f_t;
	    if(self->f_t_1 == 0.0) {
		f_t = -sample_rate * (cos(2.0 * M_PI * f * 1.0 / sample_rate)
				      - cos(2.0 * M_PI * f * 0.0 / sample_rate)) / (2.0 * M_PI * f);
	    } else if(self->f_t_2 == 0.0) {
		f_t = -sample_rate * (cos(2.0 * M_PI * f * 2.0 / sample_rate)
				      - cos(2.0 * M_PI * f * 1.0 / sample_rate)) / (2.0 * M_PI * f);
	    } else {
		f_t = self->f_t_1 * 2.0 * cos(2.0 * M_PI * f / sample_rate) - self->f_t_2;
	    }

	    self->f_t_2 = self->f_t_1;
	    self->f_t_1 = f_t;
	    out_buffer[i] = (float) f_t;
	}
    }
    return 0;
}

int cs_sine_init(cs_sine_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_synth_init((cs_synth_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }
    r = jack_set_process_callback(self->client, cs_sine_process, self);
    if(r != 0) {
	cs_synth_destroy((cs_synth_t *) self);
	return r;
    }
    self->f_t_1 = 0.0;
    self->f_t_2 = 0.0;
    r = jack_activate(self->client);
    if(r != 0) {
	cs_synth_destroy((cs_synth_t *) self);
	return r;
    }
    return 0;
}
