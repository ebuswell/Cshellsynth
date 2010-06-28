#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/triangle.h"
#include "cshellsynth/synth.h"
#include "cshellsynth/jclient.h"
#include "atomic-float.h"

static int cs_triangle_process(jack_nframes_t nframes, void *arg) {
    cs_triangle_t *self = (cs_triangle_t *) arg;
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
	    self->offset = 0.25;
	    out_buffer[i] = 0.0f;
	} else {
	    // /\
	    //   \/
	    // 0123
	    //  /\
	    // /  \
	    // 3012
	    if(self->offset >= 1.0) {
		self->offset -= 1.0;
	    }
	    float a = (float) (4.0 * self->offset);
	    if(a > 2.0f) {
		a = 4.0f - a;
	    }
	    out_buffer[i] = a - 1.0f;
	    self->offset += ((double) f);
	}
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
    self->offset = 0.25;
    r = jack_activate(self->client);
    if(r != 0) {
	cs_synth_destroy((cs_synth_t *) self);
	return r;
    }
    return 0;
}
