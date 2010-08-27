/*
 * distortion.c
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
#include <cshellsynth/distortion.h>
#include "jackruby.h"
#include "filters.h"
#include "filter.h"

static VALUE cCSDistortion;

static void cs_distort_free(void *mem) {
    cs_distort_t *cself = (cs_distort_t *) mem;
    cs_distort_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_distort_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "distort";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_distort_t *cself = ALLOC(cs_distort_t);
    int r = cs_distort_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_distort_free, cself);
}

static VALUE rbcs_distort_set_kind(VALUE self, VALUE kind) {
    cs_distort_t *cself;
    Data_Get_Struct(self, cs_distort_t, cself);
    cs_distort_set_kind(cself, NUM2INT(kind));
    return kind;
}

static VALUE rbcs_distort_gain(VALUE self) {
    cs_distort_t *cself;
    Data_Get_Struct(self, cs_distort_t, cself);
    VALUE gain_port = rb_iv_get(self, "@gain_port");
    if(NIL_P(gain_port)) {
	gain_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->gain_port);
	rb_iv_set(self, "@gain_port", gain_port);
    }
    return gain_port;
}

static VALUE rbcs_distort_set_gain(VALUE self, VALUE gain) {
    cs_distort_t *cself;
    Data_Get_Struct(self, cs_distort_t, cself);
    if(KIND_OF(gain, rb_cNumeric)) {
	cs_distort_set_gain(cself, NUM2DBL(gain));
    } else {
	VALUE gain_port = rb_iv_get(self, "@gain_port");
	if(NIL_P(gain_port)) {
	    gain_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->gain_port);
	    rb_iv_set(self, "@gain_port", gain_port);
	}
	jr_client_connect(self, gain, gain_port);
	// ignore return value
	cs_distort_set_gain(cself, NAN);
    }
    return gain;
}

static VALUE rbcs_distort_set_sharpness(VALUE self, VALUE sharpness) {
    cs_distort_t *cself;
    Data_Get_Struct(self, cs_distort_t, cself);
    cs_distort_set_sharpness(cself, NUM2DBL(sharpness));
    return sharpness;
}

void Init_distortion() {
    cCSDistortion = rb_define_class_under(mCSFilters, "LLDistortion", cCSFilter);

    rb_define_singleton_method(cCSDistortion, "new", rbcs_distort_new, -1);
    rb_define_method(cCSDistortion, "gain", rbcs_distort_gain, 0);
    rb_define_method(cCSDistortion, "gain=", rbcs_distort_set_gain, 1);
    rb_define_method(cCSDistortion, "sharpness=", rbcs_distort_set_sharpness, 1);
    rb_define_method(cCSDistortion, "kind=", rbcs_distort_set_kind, 1);
    rb_define_const(cCSDistortion, "Exponential", INT2NUM(CS_EXP));
    rb_define_const(cCSDistortion, "Hyperbolic", INT2NUM(CS_HYP));
    rb_define_const(cCSDistortion, "Arctangent", INT2NUM(CS_ATAN));
    rb_define_const(cCSDistortion, "Tube", INT2NUM(CS_TUBE));
}
