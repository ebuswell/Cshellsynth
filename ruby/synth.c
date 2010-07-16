/*
 * synth.c
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
#include <cshellsynth/synth.h>
#include "jackruby.h"
#include "synths.h"
#include "synth.h"

VALUE cCSSynth;

static VALUE rbcs_synth_freq(VALUE self) {
    cs_synth_t *cself;
    Data_Get_Struct(self, cs_synth_t, cself);
    VALUE freq_port = rb_iv_get(self, "@freq_port");
    if(NIL_P(freq_port)) {
	freq_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->freq_port);
	rb_iv_set(self, "@freq_port", freq_port);
    }
    return freq_port;
}

static VALUE rbcs_synth_set_freq(VALUE self, VALUE freq) {
    cs_synth_t *cself;
    Data_Get_Struct(self, cs_synth_t, cself);
    if(KIND_OF(freq, rb_cNumeric)) {
	cs_synth_set_freq(cself, NUM2DBL(freq));
    } else {
	VALUE freq_port = rb_iv_get(self, "@freq_port");
	if(NIL_P(freq_port)) {
	    freq_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->freq_port);
	    rb_iv_set(self, "@freq_port", freq_port);
	}
	jr_client_connect(self, freq, freq_port);
	// ignore return value
	cs_synth_set_freq(cself, NAN);
    }
    return freq;
}

static VALUE rbcs_synth_set_offset(VALUE self, VALUE offset) {
    cs_synth_t *cself;
    Data_Get_Struct(self, cs_synth_t, cself);
    cs_synth_set_offset(cself, NUM2DBL(offset));
    return offset;
}

static VALUE rbcs_synth_set_amp(VALUE self, VALUE amp) {
    cs_synth_t *cself;
    Data_Get_Struct(self, cs_synth_t, cself);
    cs_synth_set_amp(cself, NUM2DBL(amp));
    return amp;
}

static VALUE rbcs_synth_out(VALUE self) {
    VALUE out_port = rb_iv_get(self, "@out_port");
    if(NIL_P(out_port)) {
	cs_synth_t *cself;
	Data_Get_Struct(self, cs_synth_t, cself);
	out_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->out_port);
	rb_iv_set(self, "@out_port", out_port);
    }
    return out_port;
}

static VALUE rbcs_synth_set_out(VALUE self, VALUE out) {
    cs_synth_t *cself;
    Data_Get_Struct(self, cs_synth_t, cself);
    VALUE out_port = rb_iv_get(self, "@out_port");
    if(NIL_P(out_port)) {
	out_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->out_port);
	rb_iv_set(self, "@out_port", out_port);
    }
    jr_client_connect(self, out_port, out);
    // ignore return value
    return out;
}

static void cs_synth_free(void *mem) {
    cs_synth_t *cself = (cs_synth_t *) mem;
    cs_synth_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_synth_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname, rflags, rservername;
    rb_scan_args(argc, argv, "21", &rname, &rflags, &rservername);
    char *name = StringValueCStr(rname);
    jack_options_t flags = NUM2JACKOPTIONST(rflags);
    char *servername = NIL_P(rservername) ? NULL : StringValueCStr(rservername);
    cs_synth_t *cself = ALLOC(cs_synth_t);
    int r = cs_synth_init(cself, name, flags, servername);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_synth_free, cself);
}

void Init_synth() {
    cCSSynth = rb_define_class_under(mCSSynths, "Synth", cJackClient);

    rb_define_singleton_method(cCSSynth, "new", rbcs_synth_new, -1);
    rb_define_method(cCSSynth, "freq", rbcs_synth_freq, 0);
    rb_define_method(cCSSynth, "freq=", rbcs_synth_set_freq, 1);
    rb_define_method(cCSSynth, "out", rbcs_synth_out, 0);
    rb_define_method(cCSSynth, "out=", rbcs_synth_set_out, 1);
    rb_define_method(cCSSynth, "offset=", rbcs_synth_set_offset, 1);
    rb_define_method(cCSSynth, "amp=", rbcs_synth_set_amp, 1);
    rb_define_method(cCSSynth, "amplitude=", rbcs_synth_set_amp, 1);
}
