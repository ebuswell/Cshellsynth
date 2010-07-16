/*
 * mixer.c
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
#include <cshellsynth/mixer.h>
#include <math.h>
#include "jackruby.h"

VALUE cCSMixer;

static VALUE rbcs_mix_in1(VALUE self) {
    cs_mix_t *cself;
    Data_Get_Struct(self, cs_mix_t, cself);
    VALUE in1_port = rb_iv_get(self, "@in1_port");
    if(NIL_P(in1_port)) {
	in1_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->in1_port);
	rb_iv_set(self, "@in1_port", in1_port);
    }
    return in1_port;
}

static VALUE rbcs_mix_set_in1(VALUE self, VALUE in1) {
    cs_mix_t *cself;
    Data_Get_Struct(self, cs_mix_t, cself);
    if(KIND_OF(in1, rb_cNumeric)) {
	cs_mix_set_in1(cself, NUM2DBL(in1));
    } else {
	VALUE in1_port = rb_iv_get(self, "@in1_port");
	if(NIL_P(in1_port)) {
	    in1_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->in1_port);
	    rb_iv_set(self, "@in1_port", in1_port);
	}
	jr_client_connect(self, in1, in1_port);
	// ignore return value
	cs_mix_set_in1(cself, NAN);
    }
    return in1;
}

static VALUE rbcs_mix_in2(VALUE self) {
    cs_mix_t *cself;
    Data_Get_Struct(self, cs_mix_t, cself);
    VALUE in2_port = rb_iv_get(self, "@in2_port");
    if(NIL_P(in2_port)) {
	in2_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->in2_port);
	rb_iv_set(self, "@in2_port", in2_port);
    }
    return in2_port;
}

static VALUE rbcs_mix_set_in2(VALUE self, VALUE in2) {
    cs_mix_t *cself;
    Data_Get_Struct(self, cs_mix_t, cself);
    if(KIND_OF(in2, rb_cNumeric)) {
	cs_mix_set_in2(cself, NUM2DBL(in2));
    } else {
	VALUE in2_port = rb_iv_get(self, "@in2_port");
	if(NIL_P(in2_port)) {
	    in2_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->in2_port);
	    rb_iv_set(self, "@in2_port", in2_port);
	}
	jr_client_connect(self, in2, in2_port);
	// ignore return value
	cs_mix_set_in1(cself, NAN);
    }
    return in2;
}

static VALUE rbcs_mix_out(VALUE self) {
    VALUE out_port = rb_iv_get(self, "@out_port");
    if(NIL_P(out_port)) {
	cs_mix_t *cself;
	Data_Get_Struct(self, cs_mix_t, cself);
	out_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->out_port);
	rb_iv_set(self, "@out_port", out_port);
    }
    return out_port;
}

static VALUE rbcs_mix_set_out(VALUE self, VALUE out) {
    cs_mix_t *cself;
    Data_Get_Struct(self, cs_mix_t, cself);
    VALUE out_port = rb_iv_get(self, "@out_port");
    if(NIL_P(out_port)) {
	out_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->out_port);
	rb_iv_set(self, "@out_port", out_port);
    }
    jr_client_connect(self, out_port, out);
    // ignore return value
    return out;
}

static void cs_mix_free(void *mem) {
    cs_mix_t *cself = (cs_mix_t *) mem;
    cs_mix_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_mix_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "mix";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_mix_t *cself = ALLOC(cs_mix_t);
    int r = cs_mix_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_mix_free, cself);
}

void Init_mixer() {
    cCSMixer = rb_define_class("Mixer", cJackClient);

    rb_define_singleton_method(cCSMixer, "new", rbcs_mix_new, -1);
    rb_define_method(cCSMixer, "in1", rbcs_mix_in1, 0);
    rb_define_method(cCSMixer, "in1=", rbcs_mix_set_in1, 1);
    rb_define_method(cCSMixer, "in2", rbcs_mix_in2, 0);
    rb_define_method(cCSMixer, "in2=", rbcs_mix_set_in2, 1);
    rb_define_method(cCSMixer, "out", rbcs_mix_out, 0);
    rb_define_method(cCSMixer, "out=", rbcs_mix_set_out, 1);
}
