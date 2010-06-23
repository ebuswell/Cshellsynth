#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/modulator.h"
#include "cshellsynth/mixer.h"
#include "atomic-float.h"

static int cs_modu_process(jack_nframes_t nframes, void *arg) {
    cs_modu_t *self = (cs_modu_t *) arg;
    float *in1_buffer;
    float *in2_buffer;
    float *out_buffer = (float *)jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    float in1 = atomic_float_read(&self->in1);
    if(isnanf(in1)) {
	in1_buffer = (float *)jack_port_get_buffer(self->in1_port, nframes);
	if(in1_buffer == NULL) {
	    return -1;
	}
    }
    float in2 = atomic_float_read(&self->in2);
    if(isnanf(in2)) {
	in2_buffer = (float *)jack_port_get_buffer(self->in2_port, nframes);
	if(in2_buffer == NULL) {
	    return -1;
	}
    }
    int i;
    for(i = 0; i < nframes; i++) {
	out_buffer[i] = (isnanf(in1) ? in1_buffer[i] : in1) * (isnanf(in2) ? in2_buffer[i] : in2);
    }
    return 0;
}

int cs_modu_init(cs_modu_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_mix_subclass_init((cs_mix_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    r = jack_set_process_callback(self->client, cs_modu_process, self);
    if(r != 0) {
	cs_mix_destroy((cs_mix_t *) self);
	return r;
    }

    r = jack_activate(self->client);
    if(r != 0) {
	cs_mix_destroy((cs_mix_t *) self);
	return r;
    }

    return 0;
}
