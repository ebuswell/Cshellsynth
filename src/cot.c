#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/cot.h"
#include "cshellsynth/synth.h"
#include "cshellsynth/jclient.h"
#include "atomic-float.h"

#define COT_SCALE 32
#define COT_HARDNESS (4)

static int cs_cot_process(jack_nframes_t nframes, void *arg) {
    cs_cot_t *self = (cs_cot_t *) arg;
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
	    self->offset = 0.0;
	    out_buffer[i] = 0.0f;
	} else {
	    double period = sample_rate / f;
	    while(self->offset >= period) {
		self->offset -= period;
	    }
	    double c = sample_rate * 
		(log(fabs(sin((M_PI * f * (self->offset + 1)) / sample_rate)))
		 - log(fabs(sin((M_PI * f * self->offset) / sample_rate))))
		/ (M_PI * f);
	    c /= COT_SCALE;
	    c = c >= 0 ? 1 - (1/(log(exp(COT_HARDNESS) + 1)))*log(exp(-COT_HARDNESS*(c - 1)) + 1)
		: (1/(log(exp(COT_HARDNESS) + 1)))*log(exp(COT_HARDNESS*(c + 1)) + 1) - 1;
	    out_buffer[i] = (float) c;
	    self->offset += 1.0;
	}
    }
    return 0;
}

int cs_cot_init(cs_cot_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_synth_init((cs_synth_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }
    r = jack_set_process_callback(self->client, cs_cot_process, self);
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
