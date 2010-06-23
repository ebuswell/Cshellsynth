#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/clock.h"
#include "cshellsynth/jclient.h"
#include "atomic-double.h"

void cs_clock_set_meter(cs_clock_t *self, double meter) {
    atomic_double_set(&self->max, meter);
}

void cs_clock_set_bpm(cs_clock_t *self, double bpm) {
    atomic_double_set(&self->step, bpm / (60.0 * ((double) jack_get_sample_rate(self->client))));
}

static int cs_clock_process(jack_nframes_t nframes, void *arg) {
    cs_clock_t *self = (cs_clock_t *) arg;
    float *clock_buffer = (float *)jack_port_get_buffer(self->clock_port, nframes);
    if(clock_buffer == NULL) {
	return -1;
    }
    double max = atomic_double_read(&self->max);
    double step = atomic_double_read(&self->step);
    int i;
    for(i = 0; i < nframes; i++) {
	while(self->current >= max) {
	    self->current -= max;
	}
	clock_buffer[i] = (float) self->current;
	self->current += step;
    }
    return 0;
}

int cs_clock_init(cs_clock_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = jclient_init((jclient_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->clock_port = jack_port_register(self->client, "clock", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(self->clock_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return -1;
    }

    self->current = 0.0;
    atomic_double_set(&self->max, 4.0);
    atomic_double_set(&self->step, 120.0 / (60.0 * ((double) jack_get_sample_rate(self->client))));

    r = jack_set_process_callback(self->client, cs_clock_process, self);
    if(r != 0) {
	jclient_destroy((jclient_t *) self);
	return r;
    }

    r = jack_activate(self->client);
    if(r != 0) {
	jclient_destroy((jclient_t *) self);
	return r;
    }

    return 0;
}
