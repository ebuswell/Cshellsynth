/*
 * dsf.c
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
#include <cshellsynth/dsf.h>
#include <math.h>
#include "jackruby.h"
#include "synths.h"
#include "synth.h"

static VALUE cCSDsf;

static void cs_dsf_free(void *mem) {
    cs_dsf_t *cself = (cs_dsf_t *) mem;
    cs_dsf_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_dsf_bright(VALUE self) {
    cs_dsf_t *cself;
    Data_Get_Struct(self, cs_dsf_t, cself);
    VALUE bright_port = rb_iv_get(self, "@bright_port");
    if(NIL_P(bright_port)) {
	bright_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->bright_port);
	rb_iv_set(self, "@bright_port", bright_port);
    }
    return bright_port;
}

static VALUE rbcs_dsf_set_bright(VALUE self, VALUE bright) {
    cs_dsf_t *cself;
    Data_Get_Struct(self, cs_dsf_t, cself);
    if(KIND_OF(bright, rb_cNumeric)) {
	cs_dsf_set_bright(cself, NUM2DBL(bright));
    } else {
	VALUE bright_port = rb_iv_get(self, "@bright_port");
	if(NIL_P(bright_port)) {
	    bright_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->bright_port);
	    rb_iv_set(self, "@bright_port", bright_port);
	}
	jr_client_connect(self, bright, bright_port);
	// ignore return value
	cs_dsf_set_bright(cself, NAN);
    }
    return bright;
}

static VALUE rbcs_dsf_set_scale(VALUE self, VALUE scale) {
    cs_dsf_t *cself;
    Data_Get_Struct(self, cs_dsf_t, cself);
    cs_dsf_set_scale(cself, IS_TRUE(scale));
    return scale;
}

static VALUE rbcs_dsf_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "dsf";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_dsf_t *cself = ALLOC(cs_dsf_t);
    int r = cs_dsf_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_dsf_free, cself);
}

void Init_dsf() {
    cCSDsf = rb_define_class_under(mCSSynths, "LLDsf", cCSSynth);

    rb_define_singleton_method(cCSDsf, "new", rbcs_dsf_new, -1);
    rb_define_method(cCSDsf, "bright", rbcs_dsf_bright, 0);
    rb_define_method(cCSDsf, "bright=", rbcs_dsf_set_bright, 1);
    rb_define_method(cCSDsf, "scale=", rbcs_dsf_set_scale, 1);
}
