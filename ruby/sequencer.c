/*
 * sequencer.c
 * 
 * Copyright 2010 Evan Buswell
 * 
 * This file is part of Cshellsynth.
 * 
 * Cshellsynth is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Cshellsynth is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Cshellsynth.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <ruby.h>
#include <cshellsynth/sequencer.h>
#include "jackruby.h"
#include "controllers.h"
#include "controller.h"

static VALUE cCSSequencer;

static void rbcs_seq_make_sequence(VALUE self, VALUE roffset, VALUE rlength, VALUE rsequence, bool repeat) {
    cs_seq_t *cself;
    Data_Get_Struct(self, cs_seq_t, cself);

    jack_default_audio_sample_t offset = NUM2DBL(roffset);
    jack_default_audio_sample_t length = NUM2DBL(rlength);
    size_t sequence_length = RARRAY_LEN(rsequence);
    void *sequence = malloc(sizeof(float *) * (sequence_length + 1) + sizeof(float) * 3 * sequence_length);
    
    int i;
    for(i = 0; i < sequence_length; i++) {
	((float **) sequence)[i] = (float *) (sequence + sizeof(float *) * (sequence_length + 1) + sizeof(float) * 3 * i);
    }
    ((float **) sequence)[sequence_length] = NULL;
    for(i = 0; i < sequence_length; i++) {
	VALUE rsubsequence = RARRAY_PTR(rsequence)[i];
	if((TYPE(rsubsequence) != T_ARRAY)
	   || (RARRAY_LEN(rsubsequence) != 3)) {
	    free(sequence);
	    rb_raise(rb_eArgError, "sequence must consist of 3-element tuples");
	}
	((float **) sequence)[i][0] = NUM2DBL(RARRAY_PTR(rsubsequence)[0]);
	((float **) sequence)[i][1] = NUM2DBL(RARRAY_PTR(rsubsequence)[1]);
	((float **) sequence)[i][2] = NUM2DBL(RARRAY_PTR(rsubsequence)[2]);
    }
    int r = cs_seq_make_sequence_ll(cself, offset, length, sequence, repeat);
    if(r != 0) {
	rb_raise(eJackFailure, "Overall operation failed: %d", r);
    }
}

static VALUE rbcs_seq_sequence(int argc, VALUE *argv, VALUE self) {
    VALUE roffset, rlength, rsequence;
    rb_scan_args(argc, argv, "2*", &roffset, &rlength, &rsequence);
    rbcs_seq_make_sequence(self, roffset, rlength, rsequence, true);
    return self;
}

static VALUE rbcs_seq_sequence_once(int argc, VALUE *argv, VALUE self) {
    VALUE roffset, rlength, rsequence;
    rb_scan_args(argc, argv, "2*", &roffset, &rlength, &rsequence);
    rbcs_seq_make_sequence(self, roffset, rlength, rsequence, true);
    return self;
}

static void cs_seq_free(void *mem) {
    cs_seq_t *cself = (cs_seq_t *) mem;
    cs_seq_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_seq_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "seq";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_seq_t *cself = ALLOC(cs_seq_t);
    int r = cs_seq_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_seq_free, cself);
}

static VALUE rbcs_seq_clock(VALUE self) {
    VALUE clock_port = rb_iv_get(self, "@clock_port");
    if(NIL_P(clock_port)) {
	cs_seq_t *cself;
	Data_Get_Struct(self, cs_seq_t, cself);
	clock_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->clock_port);
	rb_iv_set(self, "@clock_port", clock_port);
    }
    return clock_port;
}

static VALUE rbcs_seq_set_clock(VALUE self, VALUE port) {
    VALUE clock_port = rb_iv_get(self, "@clock_port");
    if(NIL_P(clock_port)) {
	cs_seq_t *cself;
	Data_Get_Struct(self, cs_seq_t, cself);
	clock_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->clock_port);
	rb_iv_set(self, "@clock_port", clock_port);
    }
    jr_client_connect(self, port, clock_port);
    // ignore return value
    return port;
}

void Init_sequencer() {
    cCSSequencer = rb_define_class_under(mCSControllers, "Sequencer", cCSController);

    rb_define_singleton_method(cCSSequencer, "new", rbcs_seq_new, -1);
    rb_define_method(cCSSequencer, "clock", rbcs_seq_clock, 0);
    rb_define_method(cCSSequencer, "clock=", rbcs_seq_set_clock, 1);
    rb_define_method(cCSSequencer, "sequence", rbcs_seq_sequence, -1);
    rb_define_method(cCSSequencer, "sequence_once", rbcs_seq_sequence_once, -1);
}
