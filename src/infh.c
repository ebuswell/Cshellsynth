#include <jack/jack.h>
#include <math.h>
#include "infh.h"
#include "synth.h"
#include "jclient.h"

static int cs_infh_process(jack_nframes_t nframes, void *arg) {
    cs_infh_t *self = (cs_infh_t *) arg;
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
	    double f = (double) (isnanf(self->freq) ? freq_buffer[i] : self->freq);
	    if(f == 0.0 || isnan(f)) {
		self->offset = 0.0;
		out_buffer[i] = 0.0f;
	    } else {
		double period = sample_rate / f;
		double n = floor(period / 2.0); // floor((sample_rate/2) / f)
		n *= 2.0;

		while(self->offset >= period) {
		    self->offset -= period;
		}

		// 1 ( sin((n + 1)((w/2)t + (3pi/4)        )
                // - (------------------------------  -  1 )
		// n (      sin((w/2)t + 3pi/4)            )

		double wt = M_PI * f * self->offset / sample_rate;

		out_buffer[i] = (jack_default_audio_sample_t) (1.0/n)*((sin((n + 1.0)*(wt + (3.0*M_PI)/4.0))// + (3.0*n - 1.0)*M_PI_4
									/ sin(wt + ((3.0*M_PI)/4.0)))
								       - 1.0);
//		out_buffer[i] = sin(M_2_PI*f*self->offset);//(1./(2.*n))*(sin((2.*n + 1)*M_PI*f*self->offset)/sin(M_PI*f*self->offset) - 1.);
		self->offset += 1.0;
	    }
	}
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

int cs_infh_init(cs_infh_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_synth_init((cs_synth_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }
    r = jack_set_process_callback(self->client, cs_infh_process, self);
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
