#include <jack/jack.h>
#include "sine.h"
#include "synth.h"

static int cs_sine_process(jack_nframes_t nframes, void *arg) {
    cs_sine_t *self = (cs_sine_t *) arg;
    jack_default_audio_sample_t *freq_buffer = (jack_default_audio_sample_t *) jack_port_get_buffer(self->freq_port, nframes);
    if(freq_buffer == NULL) {
	return -1;
    }
    jack_default_audio_sample_t *out_buffer = (jack_default_audio_sample_t *) jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    double sample_rate = (double) jack_get_sample_rate(self->client);
    int i;
    for(i = 0; i < nframes; i++) {
	double f = (double) freq_buffer[i];
	if(f == 0.0 || f == NAN) {
	    self->offset = 0.0;
	    out_buffer[i] = 0.0f;
	} else {
	    double period = sample_rate / f;
	    if(self->offset > period) {
		self->offset -= period;
	    }
	    out_buffer[i] = sinf((2.0 * M_PI * f * self->offset) / sample_rate);
	    self->offset += 1.0;
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
    self->offset = 0.0;
    return 0;
}
