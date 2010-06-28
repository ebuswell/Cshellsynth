#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/falling_saw.h"
#include "cshellsynth/synth.h"
#include "cshellsynth/jclient.h"
#include "atomic-float.h"

static int cs_fsaw_process(jack_nframes_t nframes, void *arg) {
    cs_fsaw_t *self = (cs_fsaw_t *) arg;
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
	float f = isnanf(freq) ? freq_buffer[i] : freq;
	if(f == 0.0f || isnanf(f)) {
	    self->offset = 0.5;
	    out_buffer[i] = 0.0f;
	} else {
	    //  |\
	    // \|
	    // 0 1
	    // \ |
	    //  \|
	    // 10
	    while(self->offset >= 1.0) {
		self->offset -= 1.0;
	    }
	    out_buffer[i] = 1.0f - ((float) 2.0 * self->offset);
	    self->offset += f;
	}
    }
    return 0;
}

int cs_fsaw_init(cs_fsaw_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_synth_init((cs_synth_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }
    r = jack_set_process_callback(self->client, cs_fsaw_process, self);
    if(r != 0) {
	cs_synth_destroy((cs_synth_t *) self);
	return r;
    }
    self->offset = 0.5;
    r = jack_activate(self->client);
    if(r != 0) {
	cs_synth_destroy((cs_synth_t *) self);
	return r;
    }
    return 0;
}
