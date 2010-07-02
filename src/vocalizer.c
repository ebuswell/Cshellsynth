#include <jack/jack.h>
#include <stdlib.h>
#include <math.h>
#include <sndfile.h>
#include <alloca.h>
#include <string.h>
#include <fftw3.h>
#include "cshellsynth/vocalizer.h"
#include "cshellsynth/jclient.h"
#include "atomic-ptr.h"

static int cs_vocalizer_process(jack_nframes_t nframes, void *arg) {
    cs_vocalizer_t *self = (cs_vocalizer_t *) arg;
    float *ctl_buffer = (float *) jack_port_get_buffer(self->ctl_port, nframes);
    if(ctl_buffer == NULL) {
	return -1;
    }
    float *out_buffer = (float *) jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    cs_vocalizer_buffer_t *buffer = atomic_ptr_read(&self->buffer);
    if(buffer != NULL) {
	int i;
	for(i = 0; i < nframes; i++) {
	    if(ctl_buffer[i] == -1.0f) {
		self->playing = false;
	    } else if(ctl_buffer[i] == 1.0f) {
		self->playing = true;
		buffer->offset = 0;
	    }
	    if(self->playing) {
		out_buffer[i] = buffer->buffer[buffer->offset];
		buffer->offset++;
		if(buffer->offset == buffer->length) {
		    self->playing = false;
		}
	    } else {
		out_buffer[i] = 0.0f;
	    }
	}
    } else {
	memset(out_buffer, 0, sizeof(float) * nframes);
    }
    return 0;
}

#include "firwindow.c"

#define GRANULARITY (1024 * 1024 * 4)

int cs_vocalizer_load(cs_vocalizer_t *self, char *path) {
    SNDFILE *sf;
    SF_INFO sf_info;
    int r;
    memset(&sf_info, 0, sizeof(sf_info));
    sf = sf_open(path, SFM_READ, &sf_info);
    if(sf == NULL) {
	return -1;
    }
    if(sf_info.samplerate != jack_get_sample_rate(self->client)) {
	r = sf_close(sf);
	if(r != 0) {
	    return r;
	}
	return -1;
    }
    if(sf_info.channels != 2) {
	r = sf_close(sf);
	if(r != 0) {
	    return r;
	}
	return -1;
    }
    int bufsize;
    for(bufsize = 0; bufsize < sf_info.frames; bufsize += GRANULARITY);
    float *interleaved = malloc(bufsize * sizeof(float) * 2);
    if(interleaved == NULL) {
	return -1;
    }
    memset(interleaved, 0, bufsize * sizeof(float) * 2);

    sf_count_t c = sf_readf_float(sf, interleaved, bufsize);
    if(c < 0) {
	free(interleaved);
	return c;
    }
    c = sf_close(sf);
    if(c != 0) {
	free(interleaved);
	return c;
    }

    double *L = malloc(bufsize * sizeof(double));
    if(L == NULL) {
	free(interleaved);
	return -1;
    }
    double *R = malloc(bufsize * sizeof(double));
    if(R == NULL) {
	free(L);
	free(interleaved);
	return -1;
    }
    int i;
    for(i = 0; i < bufsize; i++) {
	L[i] = interleaved[2*i];
	R[i] = interleaved[2*i + 1];
    }
    free(interleaved);
    double *L_a = fftw_malloc(GRANULARITY * sizeof(double));
    if(L_a == NULL) {
	free(L);
	free(R);
	return -1;
    }
    double *L_f = fftw_malloc(GRANULARITY * sizeof(double));
    if(L_f == NULL) {
	free(L);
	free(R);
	fftw_free(L_a);
	return -1;
    }
    double *R_a = fftw_malloc(GRANULARITY * sizeof(double));
    if(R_a == NULL) {
	free(L);
	free(R);
	fftw_free(L_f);
	fftw_free(L_a);
	return -1;
    }
    double *R_f = fftw_malloc(GRANULARITY * sizeof(double));
    if(R_f == NULL) {
	free(L);
	free(R);
	fftw_free(L_f);
	fftw_free(L_a);
	fftw_free(R_a);
	return -1;
    }
    float *out = malloc(bufsize * sizeof(float));
    if(out == NULL) {
	free(L);
	free(R);
	fftw_free(L_f);
	fftw_free(L_a);
	fftw_free(R_f);
	fftw_free(R_a);
    }
    memset(out, 0, bufsize * sizeof(float));
    fftw_plan L_af = fftw_plan_r2r_1d(GRANULARITY, L_a, L_f, FFTW_R2HC, FFTW_MEASURE);
    fftw_plan L_fa = fftw_plan_r2r_1d(GRANULARITY, L_f, L_a, FFTW_HC2R, FFTW_MEASURE);
    fftw_plan R_af = fftw_plan_r2r_1d(GRANULARITY, R_a, R_f, FFTW_R2HC, FFTW_MEASURE);
    int j;
    for(i = 0; i + GRANULARITY <= bufsize; i += GRANULARITY/2) {
	memcpy(L_a, L + i, GRANULARITY * sizeof(double));
	firwindow_kaiser(L_a, GRANULARITY, 0, 4, 8);
	fftw_execute(L_af);
	/* memcpy(R_a, R + i, GRANULARITY * sizeof(double)); */
	/* firwindow_kaiser(R_a, GRANULARITY, 0, 9, 8); */
	/* fftw_execute(R_af); */
	/* for(j = 0; j < GRANULARITY; j++) { */
	/*     if(L_f[j] != R_f[j]) { */
	/* 	L_f[j] == 0; */
	/*     } */
	/* } */
	fftw_execute(L_fa);
	for(j = 0; j < GRANULARITY; j++) {
	    out[i + j] += L_a[j];
	}
    }
    free(R);
    free(L);
    fftw_free(L_f);
    fftw_free(L_a);
    fftw_free(R_f);
    fftw_free(R_a);
    fftw_destroy_plan(L_af);
    fftw_destroy_plan(R_af);
    fftw_destroy_plan(L_fa);

    cs_vocalizer_buffer_t *buffer = malloc(sizeof(cs_vocalizer_buffer_t));
    if(buffer == NULL) {
	free(out);
	return -1;
    }
    buffer->offset = 0;
    buffer->length = bufsize;
    buffer->buffer = out;

    buffer = atomic_ptr_xchg(&self->buffer, buffer);
    if(buffer != NULL) {
	free(buffer->buffer);
	free(buffer);
    }
    return 0;
}

int cs_vocalizer_destroy(cs_vocalizer_t *self) {
    int r = jclient_destroy((jclient_t *) self);
    if(atomic_ptr_read(&self->buffer) != NULL) {
	free(((cs_vocalizer_buffer_t *) atomic_ptr_read(&self->buffer))->buffer);
	free(atomic_ptr_read(&self->buffer));
    }
    if(r != 0) {
	return r;
    }
    return 0;
}

int cs_vocalizer_init(cs_vocalizer_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = jclient_init((jclient_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    self->ctl_port = jack_port_register(self->client, "ctl", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->ctl_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return -1;
    }

    self->out_port = jack_port_register(self->client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(self->out_port == NULL) {
	jclient_destroy((jclient_t *) self);
	return -1;
    }

    atomic_ptr_set(&self->buffer, NULL);
    self->playing = false;

    r = jack_set_process_callback(self->client, cs_vocalizer_process, self);
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
