/*
 * envelope_generator.c
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
#include <cshellsynth/envelope_generator.h>
#include "jackruby.h"

static VALUE cCSEnvelopeGenerator;

static VALUE rbcs_envg_set_attack_t(VALUE self, VALUE value) {
    cs_envg_t *cself;
    Data_Get_Struct(self, cs_envg_t, cself);
    cs_envg_set_attack_t(cself, NUM2DBL(value));
    return self;
}

static VALUE rbcs_envg_set_decay_t(VALUE self, VALUE value) {
    cs_envg_t *cself;
    Data_Get_Struct(self, cs_envg_t, cself);
    cs_envg_set_decay_t(cself, NUM2DBL(value));
    return self;
}

static VALUE rbcs_envg_set_sustain_a(VALUE self, VALUE value) {
    cs_envg_t *cself;
    Data_Get_Struct(self, cs_envg_t, cself);
    cs_envg_set_sustain_a(cself, NUM2DBL(value));
    return self;
}

static VALUE rbcs_envg_set_release_t(VALUE self, VALUE value) {
    cs_envg_t *cself;
    Data_Get_Struct(self, cs_envg_t, cself);
    cs_envg_set_release_t(cself, NUM2DBL(value));
    return self;
}

static VALUE rbcs_envg_set_linear(VALUE self, VALUE value) {
    cs_envg_t *cself;
    Data_Get_Struct(self, cs_envg_t, cself);
    cs_envg_set_linear(cself, IS_TRUE(value));
    return self;
}

static VALUE rbcs_envg_ctl(VALUE self) {
    VALUE ctl_port = rb_iv_get(self, "@ctl_port");
    if(NIL_P(ctl_port)) {
	cs_envg_t *cself;
	Data_Get_Struct(self, cs_envg_t, cself);
	ctl_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->ctl_port);
	rb_iv_set(self, "@ctl_port", ctl_port);
    }
    return ctl_port;
}

static VALUE rbcs_envg_set_ctl(VALUE self, VALUE ctl) {
    VALUE ctl_port = rb_iv_get(self, "@ctl_port");
    if(NIL_P(ctl_port)) {
	cs_envg_t *cself;
	Data_Get_Struct(self, cs_envg_t, cself);
	ctl_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->ctl_port);
	rb_iv_set(self, "@ctl_port", ctl_port);
    }
    jr_client_connect(self, ctl, ctl_port);
    // ignore return value
    return ctl;
}

static VALUE rbcs_envg_out(VALUE self) {
    VALUE out_port = rb_iv_get(self, "@out_port");
    if(NIL_P(out_port)) {
	cs_envg_t *cself;
	Data_Get_Struct(self, cs_envg_t, cself);
	out_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->out_port);
	rb_iv_set(self, "@out_port", out_port);
    }
    return out_port;
}

static VALUE rbcs_envg_set_out(VALUE self, VALUE out) {
    cs_envg_t *cself;
    Data_Get_Struct(self, cs_envg_t, cself);
    VALUE out_port = rb_iv_get(self, "@out_port");
    if(NIL_P(out_port)) {
	out_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->out_port);
	rb_iv_set(self, "@out_port", out_port);
    }
    jr_client_connect(self, out_port, out);
    // ignore return value
    return out;
}

static void cs_envg_free(void *mem) {
    cs_envg_t *cself = (cs_envg_t *) mem;
    cs_envg_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_envg_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "envg";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_envg_t *cself = ALLOC(cs_envg_t);
    int r = cs_envg_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_envg_free, cself);
}

void Init_envelope_generator() {
    cCSEnvelopeGenerator = rb_define_class("EnvelopeGenerator", cJackClient);

    rb_define_singleton_method(cCSEnvelopeGenerator, "new", rbcs_envg_new, -1);
    rb_define_method(cCSEnvelopeGenerator, "ctl", rbcs_envg_ctl, 0);
    rb_define_method(cCSEnvelopeGenerator, "ctl=", rbcs_envg_set_ctl, 1);
    rb_define_method(cCSEnvelopeGenerator, "out", rbcs_envg_out, 0);
    rb_define_method(cCSEnvelopeGenerator, "out=", rbcs_envg_set_out, 1);
    rb_define_method(cCSEnvelopeGenerator, "attack_t=", rbcs_envg_set_attack_t, 1);
    rb_define_method(cCSEnvelopeGenerator, "decay_t=", rbcs_envg_set_decay_t, 1);
    rb_define_method(cCSEnvelopeGenerator, "sustain_a=", rbcs_envg_set_sustain_a, 1);
    rb_define_method(cCSEnvelopeGenerator, "release_t=", rbcs_envg_set_release_t, 1);
    rb_define_method(cCSEnvelopeGenerator, "linear=", rbcs_envg_set_linear, 1);
}
