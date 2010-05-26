#include <ruby.h>
#include <jack/jack.h>
#include "jackruby.h"
#include <math.h>
#include <errno.h>

static VALUE cJSequencer;

typedef struct jseq_sequence_struct {
    jack_default_audio_sample_t offset;
    jack_default_audio_sample_t length;
    jack_default_audio_sample_t **seq;
    int started;
    int repeat;
} jseq_sequence_t;

typedef struct jseq_struct {
    jack_client_t *client;
    int closed;
    pthread_mutex_t lock;
    jack_port_t *out_port;
    jack_port_t *ctl_port;
    jack_port_t *timer_port;
    jack_default_audio_sample_t last; // to detect flipping
    jack_default_audio_sample_t offset;
    jack_default_audio_sample_t **current;
    jack_default_audio_sample_t out;
    int playing;
    jseq_sequence_t *curr_seq;
    jseq_sequence_t *next_seq;
} jseq_t;

static void jseq_sequence_free(void *mem) {
//    if(mem != NULL) {
	jseq_sequence_t *seq = mem;
	jack_default_audio_sample_t **ptr;
	for(ptr = seq->seq; *ptr != NULL; ptr++) {
	    free(*ptr);
	}
	free(seq->seq);
	free(seq);
//    }
}

static int jseq_process(jack_nframes_t nframes, void *arg) {
    jseq_t *cself = (jseq_t *) arg;
    jack_default_audio_sample_t *out_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(cself->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    jack_default_audio_sample_t *ctl_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(cself->ctl_port, nframes);
    if(ctl_buffer == NULL) {
	return -1;
    }
    jack_default_audio_sample_t *timer_buffer = (jack_default_audio_sample_t *)jack_port_get_buffer(cself->timer_port, nframes);
    if(timer_buffer == NULL) {
	return -1;
    }
    int r = j_mutex_lock(&cself->lock);
    {
	if(r != 0) {
	    return r;
	}
	int i;
	for(i = 0; i < nframes; i++) {
	    jack_default_audio_sample_t time = timer_buffer[i];
	    if(cself->curr_seq == NULL) {
		if(cself->next_seq != NULL) {
		    cself->curr_seq = cself->next_seq;
		    cself->next_seq = NULL;
		} else {
		    cself->last = time;
		if(cself->playing) {
		    cself->playing = 0;
		    ctl_buffer[i] = -1.0f;
		} else {
		    ctl_buffer[i] = 0.0f;
		}
		out_buffer[i] = cself->out;
		continue;
		}
	    }
	TEST_SEQ_STARTED:
	    if(!cself->curr_seq->started) {
		if((time <= cself->curr_seq->offset) ||
		   (time < cself->last)) {
		    cself->curr_seq->started = 1;
		    cself->offset = -cself->curr_seq->offset;
		    cself->current = cself->curr_seq->seq;
		    cself->last = time;
		} else {
		    // otherwise, wait until the time is flipped.
		    cself->last = time;
		    if(cself->playing) {
			cself->playing = 0;
			ctl_buffer[i] = -1.0f;
		    } else {
			ctl_buffer[i] = 0.0f;
		    }
		    out_buffer[i] = cself->out;
		    continue;
		}
	    }
	    if(time < cself->last) {
		// we flipped; add ceil(last) to offset.
		cself->offset += ceil(cself->last);
	    }
	    cself->last = time;
	    // see if the sequence is over
	    if((time + cself->offset) >= cself->curr_seq->length) {
		if(cself->next_seq != NULL) {
		    jseq_sequence_free(cself->curr_seq);
		    cself->curr_seq = cself->next_seq;
		    cself->next_seq = NULL;
		    goto TEST_SEQ_STARTED;
		} else if(cself->curr_seq->repeat) {
		    cself->offset -= cself->curr_seq->length;
		    cself->current = cself->curr_seq->seq;
		}
	    }
	TEST_SEQ_CURRENT:
	    if(*cself->current != NULL) {
		if((time + cself->offset) > (*cself->current)[1]) {
		    cself->current++;
		    goto TEST_SEQ_CURRENT;
		} else if((time + cself->offset) >= (*cself->current)[0]) {
		    cself->out = (*cself->current)[2];
		    if(!cself->playing) {
			ctl_buffer[i] = 1.0f;
			cself->playing = 1;
		    } else {
			ctl_buffer[i] = 0.0f;
		    }
		    out_buffer[i] = cself->out;
		} else {
		    if(cself->playing) {
			cself->playing = 0;
			ctl_buffer[i] = -1.0f;
		    } else {
			ctl_buffer[i] = 0.0f;
		    }
		    out_buffer[i] = cself->out;
		}
	    } else {
		if(cself->playing) {
		    cself->playing = 0;
		    ctl_buffer[i] = -1.0f;
		} else {
		    ctl_buffer[i] = 0.0f;
		}
		out_buffer[i] = cself->out;
	    }
	}
    }
    r = j_mutex_unlock(&cself->lock);
    if(r != 0) {
	return r;
    }
    return 0;
}

static void jseq_free(void *mem) {
    jseq_t *cself = (jseq_t *) mem;
    if(!cself->closed) {
	j_client_close(cself->client);
    }
    if(cself->curr_seq != NULL) {
	jseq_sequence_free(cself->curr_seq);
    }
    if(cself->next_seq != NULL) {
	jseq_sequence_free(cself->next_seq);
    }
    pthread_mutex_destroy(&cself->lock);
    xfree(cself);
    jclient_free(cself);
}

static VALUE jseq_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "sequencer";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }

    jseq_t *cself = ALLOC(jseq_t);
    jclient_init(name, 0, NULL, (jclient_t *) cself);

    cself->out_port = j_client_port_register(cself->client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(cself->out_port == NULL) {
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    cself->ctl_port = j_client_port_register(cself->client, "ctl", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if(cself->ctl_port == NULL) {
	j_client_port_unregister(cself->client, cself->out_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    cself->timer_port = j_client_port_register(cself->client, "timer", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if(cself->timer_port == NULL) {
	j_client_port_unregister(cself->client, cself->ctl_port);
	j_client_port_unregister(cself->client, cself->out_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    int r = j_mutex_init(&cself->lock);
    if(r != 0) {
	j_client_port_unregister(cself->client, cself->timer_port);
	j_client_port_unregister(cself->client, cself->ctl_port);
	j_client_port_unregister(cself->client, cself->out_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    r = jack_set_process_callback(cself->client, jseq_process, cself);
    if(r != 0) {
	pthread_mutex_destroy(&cself->lock);
	j_client_port_unregister(cself->client, cself->timer_port);
	j_client_port_unregister(cself->client, cself->ctl_port);
	j_client_port_unregister(cself->client, cself->out_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Could not set process callback");
    }

    cself->last = 0.0f;
    cself->offset = 0.0f;
    cself->current = NULL;
    cself->curr_seq = NULL;
    cself->next_seq = NULL;
    cself->out = NAN;
    cself->playing = 0;

    r = j_client_activate(cself->client);
    if(r != 0) {
	pthread_mutex_destroy(&cself->lock);
	j_client_port_unregister(cself->client, cself->timer_port);
	j_client_port_unregister(cself->client, cself->ctl_port);
	j_client_port_unregister(cself->client, cself->out_port);
	j_client_close(cself->client);
	xfree(cself);
	rb_raise(eJackFailure, "Overall operation failed");
    }

    VALUE self = Data_Wrap_Struct(klass, 0, jseq_free, cself);
    rb_iv_set(self, "@out", Data_Wrap_Struct(cJackPort, 0, fake_free, cself->out_port));
    rb_iv_set(self, "@ctl", Data_Wrap_Struct(cJackPort, 0, fake_free, cself->ctl_port));
    rb_iv_set(self, "@timer", Data_Wrap_Struct(cJackPort, 0, fake_free, cself->timer_port));
    return self;
}

static VALUE jseq_out(VALUE self) {
    return rb_iv_get(self, "@out");
}

static VALUE jseq_ctl(VALUE self) {
    return rb_iv_get(self, "@ctl");
}

static VALUE jseq_timer(VALUE self) {
    return rb_iv_get(self, "@timer");
}

static VALUE jseq_set_timer(VALUE self, VALUE port) {
    return jclient_connect(self, port, rb_iv_get(self, "@timer"));
}

static void jseq_make_sequence(jseq_t *cself, VALUE rstart, VALUE rlength, VALUE rseq, int repeat) {
    // Build the entire thing on the stack first, in case
    // there's a type error.
    jseq_sequence_t seq;
    seq.offset = NUM2DBL(rstart);
    seq.length = NUM2DBL(rlength);
    seq.started = 0;
    seq.repeat = repeat;
    int seq_length = RARRAY_LEN(rseq);
    seq.seq = alloca(sizeof(jack_default_audio_sample_t *) * (seq_length + 1));
    int i;
    for(i = 0; i < seq_length; i++) {
	VALUE rsubseq = RARRAY_PTR(rseq)[i];
	if((TYPE(rsubseq) != T_ARRAY)
	   || (RARRAY_LEN(rsubseq) != 3)) {
	    rb_raise(rb_eArgError, "sequence must consist of 3-element tuples");
	}
	seq.seq[i] = alloca(sizeof(jack_default_audio_sample_t)*3);
	seq.seq[i][0] = NUM2DBL(RARRAY_PTR(rsubseq)[0]);
	seq.seq[i][1] = NUM2DBL(RARRAY_PTR(rsubseq)[1]);
	seq.seq[i][2] = NUM2DBL(RARRAY_PTR(rsubseq)[2]);
    }
    // Everything worked, now copy into memory;
    jseq_sequence_t *m_seq = malloc(sizeof(jseq_sequence_t));
    memcpy(m_seq, &seq, sizeof(jseq_sequence_t));
    m_seq->seq = malloc(sizeof(jack_default_audio_sample_t *) * (seq_length + 1));
    for(i = 0; i < seq_length; i++) {
	m_seq->seq[i] = malloc(sizeof(jack_default_audio_sample_t)*3);
	memcpy(m_seq->seq[i], seq.seq[i], sizeof(jack_default_audio_sample_t)*3);
    }
    m_seq->seq[seq_length] = NULL;
    int r = j_mutex_lock(&cself->lock);
    {
	if(r != 0) {
	    jseq_sequence_free(m_seq);
	    rb_raise(rb_eRuntimeError, "Could not acquire pthread mutex: %s", strerror(errno));
	}
	if(cself->curr_seq == NULL) {
	    cself->curr_seq = m_seq;
	} else if(!(cself->curr_seq->started)) {
	    jseq_sequence_free(cself->curr_seq);
	    cself->curr_seq = m_seq;
	} else {
	    if(cself->next_seq != NULL) {
		jseq_sequence_free(cself->next_seq);
	    }
	    cself->next_seq = m_seq;
	}
    }
    r = pthread_mutex_unlock(&cself->lock);
    if(r != 0) {
	rb_raise(rb_eRuntimeError, "Could not release pthread mutex: %s", strerror(errno));
    }
}

static VALUE jseq_sequence(int argc, VALUE *argv, VALUE self) {
    jseq_t *cself;
    Data_Get_Struct(self, jseq_t, cself);
    VALUE rstart, rlength, rseq;
    rb_scan_args(argc, argv, "2*", &rstart, &rlength, &rseq);
    jseq_make_sequence(cself, rstart, rlength, rseq, 1);
    return self;
}

static VALUE jseq_sequence_once(int argc, VALUE *argv, VALUE self) {
    jseq_t *cself;
    Data_Get_Struct(self, jseq_t, cself);
    VALUE rstart, rlength, rseq;
    rb_scan_args(argc, argv, "2*", &rstart, &rlength, &rseq);
    jseq_make_sequence(cself, rstart, rlength, rseq, 0);
    return self;
}

void Init_sequencer() {
    cJSequencer = rb_define_class_under(mJack, "Sequencer", cJackClient);
    
    rb_define_singleton_method(cJSequencer, "new", jseq_new, -1);
    rb_define_method(cJSequencer, "out", jseq_out, 0);
    rb_define_method(cJSequencer, "ctl", jseq_ctl, 0);
    rb_define_method(cJSequencer, "timer", jseq_timer, 0);
    rb_define_method(cJSequencer, "timer=", jseq_set_timer, 1);
    rb_define_method(cJSequencer, "sequence", jseq_sequence, -1);
    rb_define_method(cJSequencer, "sequence_once", jseq_sequence_once, -1);
}
