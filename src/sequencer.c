#include <jack/jack.h>
#include <math.h>
#include <malloc.h>
#include "cshellsynth/sequencer.h"
#include "cshellsynth/controller.h"
#include "atomic-ptr.h"

static void cs_seq_sequence_free(void *mem) {
    cs_seq_sequence_t *seq = mem;
    float **ptr;
    for(ptr = seq->seq; *ptr != NULL; ptr++) {
	free(*ptr);
    }
    free(seq->seq);
    free(seq);
}

int cs_seq_destroy(cs_seq_t *self) {
    int r = cs_ctlr_destroy((cs_ctlr_t *) self);
    if(self->curr_seq != NULL) {
	cs_seq_sequence_free(self->curr_seq);
    }
    if(atomic_ptr_read(&self->next_seq) != NULL) {
	cs_seq_sequence_free(atomic_ptr_read(&self->next_seq));
    }
    return r;
}

static int cs_seq_process(jack_nframes_t nframes, void *arg) {
    cs_seq_t *self = (cs_seq_t *) arg;
    float *out_buffer = (float *)jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    float *ctl_buffer = (float *)jack_port_get_buffer(self->ctl_port, nframes);
    if(ctl_buffer == NULL) {
	return -1;
    }

    float *clock_buffer = (float *)jack_port_get_buffer(self->clock_port, nframes);
    if(clock_buffer == NULL) {
	return -1;
    }

    int i;
    for(i = 0; i < nframes; i++) {
	float time = clock_buffer[i];
	if(self->curr_seq == NULL) {
	    self->curr_seq = atomic_ptr_xchg(&self->next_seq, NULL);
	    if(self->curr_seq == NULL) {
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
	    cs_seq_sequence_t *old_seq = self->curr_seq;
	    self->curr_seq = atomic_ptr_xchg(&self->next_seq, NULL);
	    if(self->curr_seq != NULL) {
		cs_seq_sequence_free(old_seq);
		goto TEST_SEQ_STARTED;
	    } else if(old_seq->repeat) {
		self->curr_seq = old_seq;
		self->offset -= self->curr_seq->length;
		self->current = self->curr_seq->seq;
	    } else {
		cs_seq_sequence_free(old_seq);
		continue;
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
    self->out = 0.0f;
    self->playing = false;
    self->curr_seq = NULL;
    atomic_ptr_set(&self->next_seq, NULL);

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

void cs_seq_make_sequence(cs_seq_t *self, float offset, float length, float **sequence, bool repeat) {
    cs_seq_sequence_t *seq = malloc(sizeof(cs_seq_sequence_t));
    seq->offset = offset;
    seq->length = length;
    seq->started = false;
    seq->repeat = repeat;
    int i;
    for(i = 0; sequence[i] != NULL; i++); // just detect length
    float **sequence_cpy = malloc(sizeof(float *) * (i + 1));
    sequence_cpy[i] = NULL;
    for(i = 0; sequence[i] != NULL; i++) {
	sequence_cpy[i] = malloc(sizeof(float) * 3);
	sequence_cpy[i][0] = sequence[i][0];
	sequence_cpy[i][1] = sequence[i][1];
	sequence_cpy[i][2] = sequence[i][2];
    }
    seq->seq = sequence_cpy;
    cs_seq_sequence_t *oldseq = atomic_ptr_xchg(&self->next_seq, seq);
    if(oldseq != NULL) {
	cs_seq_sequence_free(oldseq);
    }
}
