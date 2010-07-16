/*
 * noise.c
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
#include <cshellsynth/noise.h>
#include "jackruby.h"
#include "synths.h"

static VALUE cCSNoise;

static VALUE rbcs_noise_out(VALUE self) {
    VALUE out_port = rb_iv_get(self, "@out_port");
    if(NIL_P(out_port)) {
	cs_noise_t *cself;
	Data_Get_Struct(self, cs_noise_t, cself);
	out_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->out_port);
	rb_iv_set(self, "@out_port", out_port);
    }
    return out_port;
}

static VALUE rbcs_noise_set_out(VALUE self, VALUE out) {
    cs_noise_t *cself;
    Data_Get_Struct(self, cs_noise_t, cself);
    VALUE out_port = rb_iv_get(self, "@out_port");
    if(NIL_P(out_port)) {
	out_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->out_port);
	rb_iv_set(self, "@out_port", out_port);
    }
    jr_client_connect(self, out_port, out);
    // ignore return value
    return out;
}

static VALUE rbcs_noise_set_kind(VALUE self, VALUE kind) {
    cs_noise_t *cself;
    Data_Get_Struct(self, cs_noise_t, cself);
    cs_noise_set_kind(cself, NUM2INT(kind));
    return kind;
}

static VALUE rbcs_noise_set_offset(VALUE self, VALUE offset) {
    cs_noise_t *cself;
    Data_Get_Struct(self, cs_noise_t, cself);
    cs_noise_set_offset(cself, NUM2DBL(offset));
    return offset;
}

static VALUE rbcs_noise_set_amp(VALUE self, VALUE amp) {
    cs_noise_t *cself;
    Data_Get_Struct(self, cs_noise_t, cself);
    cs_noise_set_amp(cself, NUM2DBL(amp));
    return amp;
}

static void cs_noise_free(void *mem) {
    cs_noise_t *cself = (cs_noise_t *) mem;
    cs_noise_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_noise_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "noise";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_noise_t *cself = ALLOC(cs_noise_t);
    int r = cs_noise_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_noise_free, cself);
}

void Init_noise() {
    cCSNoise = rb_define_class_under(mCSSynths, "Noise", cJackClient);

    rb_define_singleton_method(cCSNoise, "new", rbcs_noise_new, -1);
    rb_define_method(cCSNoise, "out", rbcs_noise_out, 0);
    rb_define_method(cCSNoise, "out=", rbcs_noise_set_out, 1);
    rb_define_method(cCSNoise, "kind=", rbcs_noise_set_kind, 1);
    rb_define_method(cCSNoise, "offset=", rbcs_noise_set_offset, 1);
    rb_define_method(cCSNoise, "amp=", rbcs_noise_set_amp, 1);
    rb_define_method(cCSNoise, "amplitude=", rbcs_noise_set_amp, 1);
    rb_define_const(cCSNoise, "White", INT2NUM(CS_WHITE));
    rb_define_const(cCSNoise, "Pink", INT2NUM(CS_PINK));
    rb_define_const(cCSNoise, "Red", INT2NUM(CS_RED));
}
