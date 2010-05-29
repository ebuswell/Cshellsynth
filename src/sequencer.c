#include <jack/jack.h>
#include <math.h>
#include <malloc.h>
#include "sequencer.h"
#include "controller.h"

static void cs_seq_sequence_free(void *mem) {
    cs_seq_sequence_t *seq = mem;
    jack_default_audio_sample_t **ptr;
    for(ptr = seq->seq; *ptr != NULL; ptr++) {
	free(*ptr);
    }
    free(seq->seq);
    free(seq);
}

int cs_seq_destroy(cs_seq_t *self) {
    if(self->curr_seq != NULL) {
	cs_seq_sequence_free(self->curr_seq);
    }
    if(self->next_seq != NULL) {
	cs_seq_sequence_free(self->next_seq);
    }
    return cs_ctlr_destroy((cs_ctlr_t *) self);
}

static int cs_seq_process(jack_nframes_t nframes, void *arg) {
    cs_seq_t *self = (cs_seq_t *) arg;
    jack_default_audio_sample_t *out_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    jack_default_audio_sample_t *ctl_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(self->ctl_port, nframes);
    if(ctl_buffer == NULL) {
	return -1;
    }

    jack_default_audio_sample_t *clock_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(self->clock_port, nframes);
    if(clock_buffer == NULL) {
	return -1;
    }

    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    return r;
	}
	int i;
	for(i = 0; i < nframes; i++) {
	    jack_default_audio_sample_t time = clock_buffer[i];
	    if(self->curr_seq == NULL) {
		if(self->next_seq != NULL) {
		    self->curr_seq = self->next_seq;
		    self->next_seq = NULL;
		} else {
		    self->last = time;
		if(self->playing) {
		    self->playing = false;
		    ctl_buffer[i] = -1.0f;
		} else {
		    ctl_buffer[i] = 0.0f;
		}
		out_buffer[i] = self->out;
		continue;
		}
	    }
	TEST_SEQ_STARTED:
	    if(!self->curr_seq->started) {
		if((time <= self->curr_seq->offset) ||
		   (time < self->last)) {
		    self->curr_seq->started = true;
		    self->offset = -self->curr_seq->offset;
		    self->current = self->curr_seq->seq;
		    self->last = time;
		} else {
		    // otherwise, wait until the time is flipped.
		    self->last = time;
		    if(self->playing) {
			self->playing = false;
			ctl_buffer[i] = -1.0f;
		    } else {
			ctl_buffer[i] = 0.0f;
		    }
		    out_buffer[i] = self->out;
		    continue;
		}
	    }
	    if(time < self->last) {
		// we flipped; add ceil(last) to offset.
		self->offset += ceil(self->last);
	    }
	    self->last = time;
	    // see if the sequence is over
	    if((time + self->offset) >= self->curr_seq->length) {
		if(self->next_seq != NULL) {
		    cs_seq_sequence_free(self->curr_seq);
		    self->curr_seq = self->next_seq;
		    self->next_seq = NULL;
		    goto TEST_SEQ_STARTED;
		} else if(self->curr_seq->repeat) {
		    self->offset -= self->curr_seq->length;
		    self->current = self->curr_seq->seq;
		}
	    }
	TEST_SEQ_CURRENT:
	    if(*self->current != NULL) {
		if((time + self->offset) > (*self->current)[1]) {
		    self->current++;
		    goto TEST_SEQ_CURRENT;
		} else if((time + self->offset) >= (*self->current)[0]) {
		    self->out = (*self->current)[2];
		    if(!self->playing) {
			ctl_buffer[i] = 1.0f;
			self->playing = true;
		    } else {
			ctl_buffer[i] = 0.0f;
		    }
		    out_buffer[i] = self->out;
		} else {
		    if(self->playing) {
			self->playing = false;
			ctl_buffer[i] = -1.0f;
		    } else {
			ctl_buffer[i] = 0.0f;
		    }
		    out_buffer[i] = self->out;
		}
	    } else {
		if(self->playing) {
		    self->playing = false;
		    ctl_buffer[i] = -1.0f;
		} else {
		    ctl_buffer[i] = 0.0f;
		}
		out_buffer[i] = self->out;
	    }
	}
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

int cs_seq_init(cs_seq_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_ctlr_init((cs_ctlr_t *) self, client_name, flags, server_name);
    if(r != 0) {
	cs_ctlr_destroy((cs_ctlr_t *) self);
	return r;
    }

    self->clock_port = jack_port_register(self->client, "clock", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(self->clock_port == NULL) {
	cs_ctlr_destroy((cs_ctlr_t *) self);
	return -1;
    }

    self->last = 0.0f;
    self->offset = 0.0f;
    self->current = NULL;
    self->out = NAN;
    self->playing = false;
    self->curr_seq = NULL;
    self->next_seq = NULL;

    r = jack_set_process_callback(self->client, cs_seq_process, self);
    if(r != 0) {
	cs_ctlr_destroy((cs_ctlr_t *) self);
	return r;
    }

    r = jack_activate(self->client);
    if(r != 0) {
	cs_ctlr_destroy((cs_ctlr_t *) self);
	return r;
    }

    return 0;
}

int cs_seq_make_sequence(cs_seq_t *self, jack_default_audio_sample_t offset, jack_default_audio_sample_t length, jack_default_audio_sample_t **sequence, bool repeat) {
    cs_seq_sequence_t *seq = malloc(sizeof(cs_seq_sequence_t));
    seq->offset = offset;
    seq->length = length;
    seq->started = false;
    seq->repeat = repeat;
    int i;
    for(i = 0; sequence[i] != NULL; i++); // just detect length
    jack_default_audio_sample_t **sequence_cpy = malloc(sizeof(jack_default_audio_sample_t *) * (i + 1));
    sequence_cpy[i] = NULL;
    for(i = 0; sequence[i] != NULL; i++) {
	sequence_cpy[i] = malloc(sizeof(jack_default_audio_sample_t) * 3);
	sequence_cpy[i][0] = sequence[i][0];
	sequence_cpy[i][1] = sequence[i][1];
	sequence_cpy[i][2] = sequence[i][2];
    }
    seq->seq = sequence_cpy;
    int r = pthread_mutex_lock(&self->lock);
    {
	if(r != 0) {
	    cs_seq_sequence_free(seq);
	    return r;
	}
	if(self->curr_seq == NULL) {
	    self->curr_seq = seq;
	} else if(!(self->curr_seq->started)) {
	    cs_seq_sequence_free(self->curr_seq);
	    self->curr_seq = seq;
	} else {
	    if(self->next_seq != NULL) {
		cs_seq_sequence_free(self->next_seq);
	    }
	    self->next_seq = seq;
	}
    }
    r = pthread_mutex_unlock(&self->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}
