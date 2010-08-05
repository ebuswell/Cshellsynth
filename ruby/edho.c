/*
 * edho.c
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
#include <cshellsynth/edho.h>
#include <math.h>
#include "jackruby.h"
#include "synths.h"
#include "synth.h"

static VALUE cCSEdho;

static void cs_edho_free(void *mem) {
    cs_edho_t *cself = (cs_edho_t *) mem;
    cs_edho_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_edho_bright(VALUE self) {
    cs_edho_t *cself;
    Data_Get_Struct(self, cs_edho_t, cself);
    VALUE bright_port = rb_iv_get(self, "@bright_port");
    if(NIL_P(bright_port)) {
	bright_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->bright_port);
	rb_iv_set(self, "@bright_port", bright_port);
    }
    return bright_port;
}

static VALUE rbcs_edho_set_bright(VALUE self, VALUE bright) {
    cs_edho_t *cself;
    Data_Get_Struct(self, cs_edho_t, cself);
    if(KIND_OF(bright, rb_cNumeric)) {
	cs_edho_set_bright(cself, NUM2DBL(bright));
    } else {
	VALUE bright_port = rb_iv_get(self, "@bright_port");
	if(NIL_P(bright_port)) {
	    bright_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->bright_port);
	    rb_iv_set(self, "@bright_port", bright_port);
	}
	jr_client_connect(self, bright, bright_port);
	// ignore return value
	cs_edho_set_bright(cself, NAN);
    }
    return bright;
}

static VALUE rbcs_edho_set_scale(VALUE self, VALUE scale) {
    cs_edho_t *cself;
    Data_Get_Struct(self, cs_edho_t, cself);
    cs_edho_set_scale(cself, IS_TRUE(scale));
    return scale;
}

static VALUE rbcs_edho_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "edho";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_edho_t *cself = ALLOC(cs_edho_t);
    int r = cs_edho_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_edho_free, cself);
}

void Init_edho() {
    cCSEdho = rb_define_class_under(mCSSynths, "LLEdho", cCSSynth);

    rb_define_singleton_method(cCSEdho, "new", rbcs_edho_new, -1);
    rb_define_method(cCSEdho, "bright", rbcs_edho_bright, 0);
    rb_define_method(cCSEdho, "bright=", rbcs_edho_set_bright, 1);
    rb_define_method(cCSEdho, "scale=", rbcs_edho_set_scale, 1);
}
