/*
 * pan.c
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
#include <math.h>
#include <cshellsynth/pan.h>
#include "jackruby.h"

static VALUE cCSPan;

static VALUE rbcs_pan_in(VALUE self) {
    cs_pan_t *cself;
    Data_Get_Struct(self, cs_pan_t, cself);
    VALUE in_port = rb_iv_get(self, "@in_port");
    if(NIL_P(in_port)) {
	in_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->in_port);
	rb_iv_set(self, "@in_port", in_port);
    }
    return in_port;
}

static VALUE rbcs_pan_set_in(VALUE self, VALUE in) {
    VALUE in_port = rb_iv_get(self, "@in_port");
    if(NIL_P(in_port)) {
	cs_pan_t *cself;
	Data_Get_Struct(self, cs_pan_t, cself);
	in_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->in_port);
	rb_iv_set(self, "@in_port", in_port);
    }
    jr_client_connect(self, in, in_port);
    // ignore return value
    return in;
}

static VALUE rbcs_pan_pan(VALUE self) {
    cs_pan_t *cself;
    Data_Get_Struct(self, cs_pan_t, cself);
    VALUE pan_port = rb_iv_get(self, "@pan_port");
    if(NIL_P(pan_port)) {
	pan_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->pan_port);
	rb_iv_set(self, "@pan_port", pan_port);
    }
    return pan_port;
}

static VALUE rbcs_pan_set_pan(VALUE self, VALUE pan) {
    cs_pan_t *cself;
    Data_Get_Struct(self, cs_pan_t, cself);
    if(KIND_OF(pan, rb_cNumeric)) {
	cs_pan_set_pan(cself, NUM2DBL(pan));
    } else {
	VALUE pan_port = rb_iv_get(self, "@pan_port");
	if(NIL_P(pan_port)) {
	    pan_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->pan_port);
	    rb_iv_set(self, "@pan_port", pan_port);
	}
	jr_client_connect(self, pan, pan_port);
	// ignore return value
	cs_pan_set_pan(cself, NAN);
    }
    return pan;
}

static VALUE rbcs_pan_outL(VALUE self) {
    VALUE outL_port = rb_iv_get(self, "@outL_port");
    if(NIL_P(outL_port)) {
	cs_pan_t *cself;
	Data_Get_Struct(self, cs_pan_t, cself);
	outL_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->outL_port);
	rb_iv_set(self, "@outL_port", outL_port);
    }
    return outL_port;
}

static VALUE rbcs_pan_set_outL(VALUE self, VALUE outL) {
    cs_pan_t *cself;
    Data_Get_Struct(self, cs_pan_t, cself);
    VALUE outL_port = rb_iv_get(self, "@outL_port");
    if(NIL_P(outL_port)) {
	outL_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->outL_port);
	rb_iv_set(self, "@outL_port", outL_port);
    }
    jr_client_connect(self, outL_port, outL);
    // ignore return value
    return outL;
}

static VALUE rbcs_pan_outR(VALUE self) {
    VALUE outR_port = rb_iv_get(self, "@outR_port");
    if(NIL_P(outR_port)) {
	cs_pan_t *cself;
	Data_Get_Struct(self, cs_pan_t, cself);
	outR_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->outR_port);
	rb_iv_set(self, "@outR_port", outR_port);
    }
    return outR_port;
}

static VALUE rbcs_pan_set_outR(VALUE self, VALUE outR) {
    cs_pan_t *cself;
    Data_Get_Struct(self, cs_pan_t, cself);
    VALUE outR_port = rb_iv_get(self, "@outR_port");
    if(NIL_P(outR_port)) {
	outR_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->outR_port);
	rb_iv_set(self, "@outR_port", outR_port);
    }
    jr_client_connect(self, outR_port, outR);
    // ignore return value
    return outR;
}

static void cs_pan_free(void *mem) {
    cs_pan_t *cself = (cs_pan_t *) mem;
    cs_pan_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_pan_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "pan";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_pan_t *cself = ALLOC(cs_pan_t);
    int r = cs_pan_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_pan_free, cself);
}

void Init_pan() {
    cCSPan = rb_define_class("LLPan", cJackClient);

    rb_define_singleton_method(cCSPan, "new", rbcs_pan_new, -1);
    rb_define_method(cCSPan, "in", rbcs_pan_in, 0);
    rb_define_method(cCSPan, "in=", rbcs_pan_set_in, 1);
    rb_define_method(cCSPan, "pan", rbcs_pan_pan, 0);
    rb_define_method(cCSPan, "pan=", rbcs_pan_set_pan, 1);
    rb_define_method(cCSPan, "outL", rbcs_pan_outL, 0);
    rb_define_method(cCSPan, "outL=", rbcs_pan_set_outL, 1);
    rb_define_method(cCSPan, "outR", rbcs_pan_outR, 0);
    rb_define_method(cCSPan, "outR=", rbcs_pan_set_outR, 1);
}
