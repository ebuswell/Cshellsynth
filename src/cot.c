#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/cot.h"
#include "cshellsynth/synth.h"
#include "cshellsynth/jclient.h"
#include "atomic-float.h"

static int cs_cot_process(jack_nframes_t nframes, void *arg) {
    cs_cot_t *self = (cs_cot_t *) arg;
    float *freq_buffer;
    float *out_buffer = (float *) jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    float freq = atomic_float_read(&self->freq);
    if(isnanf(freq)) {
	freq_buffer = (float *) jack_port_get_buffer(self->freq_port, nframes);
	if(freq_buffer == NULL) {
	    return -1;
	}
    }
    float offset = atomic_float_read(&self->offset);
    float amp = atomic_float_read(&self->amp);
    int i;
    for(i = 0; i < nframes; i++) {
	double f = (double) (isnanf(freq) ? freq_buffer[i] : freq);
	if(f == 0.0 || isnan(f)) {
	    self->t = 0.0;
	    out_buffer[i] = offset;
	} else {
	    if(self->t >= 1.0) {
		self->t -= 1.0;
	    }
	    double out = (log(fabs(sin(M_PI * (self->t + f))
				      / sin(M_PI * self->t)))
			     / (2.0 * M_PI * f));
	    if(out == INFINITY) {
		out = HUGE;
	    } else if(out == -INFINITY) {
		out = -HUGE;
	    }
	    self->t += f;
	    out_buffer[i] = out * amp + offset;
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
    self->t = 0.0;
    r = jack_activate(self->client);
    if(r != 0) {
	cs_synth_destroy((cs_synth_t *) self);
	return r;
    }
    return 0;
}
