/*
 * bandpass.c
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
#include <cshellsynth/bandpass.h>
#include <math.h> /* for NAN */
#include "jackruby.h"
#include "filters.h"
#include "filter.h"

static VALUE cCSBandpass;

static void cs_bandpass_free(void *mem) {
    cs_bandpass_t *cself = (cs_bandpass_t *) mem;
    cs_bandpass_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_bandpass_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "bandpass";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_bandpass_t *cself = ALLOC(cs_bandpass_t);
    int r = cs_bandpass_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_bandpass_free, cself);
}

static VALUE rbcs_bandpass_freq(VALUE self) {
    cs_bandpass_t *cself;
    Data_Get_Struct(self, cs_bandpass_t, cself);
    VALUE freq_port = rb_iv_get(self, "@freq_port");
    if(NIL_P(freq_port)) {
	freq_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->freq_port);
	rb_iv_set(self, "@freq_port", freq_port);
    }
    return freq_port;
}

static VALUE rbcs_bandpass_set_freq(VALUE self, VALUE freq) {
    cs_bandpass_t *cself;
    Data_Get_Struct(self, cs_bandpass_t, cself);
    if(KIND_OF(freq, rb_cNumeric)) {
	cs_bandpass_set_freq(cself, NUM2DBL(freq));
    } else {
	VALUE freq_port = rb_iv_get(self, "@freq_port");
	if(NIL_P(freq_port)) {
	    freq_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->freq_port);
	    rb_iv_set(self, "@freq_port", freq_port);
	}
	jr_client_connect(self, freq, freq_port);
	// ignore return value
	cs_bandpass_set_freq(cself, NAN);
    }
    return freq;
}

static VALUE rbcs_bandpass_set_Q(VALUE self, VALUE Q) {
    cs_bandpass_t *cself;
    Data_Get_Struct(self, cs_bandpass_t, cself);
    cs_bandpass_set_Q(cself, NUM2DBL(Q));
    return Q;
}

static VALUE rbcs_bandpass_set_atten(VALUE self, VALUE atten) {
    cs_bandpass_t *cself;
    Data_Get_Struct(self, cs_bandpass_t, cself);
    cs_bandpass_set_atten(cself, NUM2DBL(atten));
    return atten;
}

void Init_bandpass() {
    cCSBandpass = rb_define_class_under(mCSFilters, "LLBandpass", cCSFilter);

    rb_define_singleton_method(cCSBandpass, "new", rbcs_bandpass_new, -1);
    rb_define_method(cCSBandpass, "freq", rbcs_bandpass_freq, 0);
    rb_define_method(cCSBandpass, "freq=", rbcs_bandpass_set_freq, 1);
    rb_define_method(cCSBandpass, "Q=", rbcs_bandpass_set_Q, 1);
    rb_define_method(cCSBandpass, "atten=", rbcs_bandpass_set_atten, 1);
}
