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
    int i;
    for(i = 0; i < nframes; i++) {
	double f = (double) (isnanf(freq) ? freq_buffer[i] : freq);
	if(f == 0.0 || isnan(f)) {
	    self->offset = 0.0;
	    out_buffer[i] = 0.0f;
	} else {
	    out_buffer[i] = (float) (log(fabs(cos(M_PI * f)
					      + sin(M_PI * f) / tan(M_PI * self->offset))) / (2.0 * M_PI * f));
	    if(out_buffer[i] == INFINITY) {
		out_buffer[i] = HUGE;
	    } else if(out_buffer[i] == -INFINITY) {
		out_buffer[i] = -HUGE;
	    }
	    self->offset += f;
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
