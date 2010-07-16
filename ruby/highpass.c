/*
 * highpass.c
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
#include <cshellsynth/highpass.h>
#include <math.h> /* for NAN */
#include "jackruby.h"
#include "filters.h"
#include "filter.h"

static VALUE cCSHighpass;

static void cs_highpass_free(void *mem) {
    cs_highpass_t *cself = (cs_highpass_t *) mem;
    cs_highpass_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_highpass_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "highpass";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_highpass_t *cself = ALLOC(cs_highpass_t);
    int r = cs_highpass_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_highpass_free, cself);
}

static VALUE rbcs_highpass_freq(VALUE self) {
    cs_highpass_t *cself;
    Data_Get_Struct(self, cs_highpass_t, cself);
    VALUE freq_port = rb_iv_get(self, "@freq_port");
    if(NIL_P(freq_port)) {
	freq_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->freq_port);
	rb_iv_set(self, "@freq_port", freq_port);
    }
    return freq_port;
}

static VALUE rbcs_highpass_set_freq(VALUE self, VALUE freq) {
    cs_highpass_t *cself;
    Data_Get_Struct(self, cs_highpass_t, cself);
    if(KIND_OF(freq, rb_cNumeric)) {
	cs_highpass_set_freq(cself, NUM2DBL(freq));
    } else {
	VALUE freq_port = rb_iv_get(self, "@freq_port");
	if(NIL_P(freq_port)) {
	    freq_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->freq_port);
	    rb_iv_set(self, "@freq_port", freq_port);
	}
	jr_client_connect(self, freq, freq_port);
	// ignore return value
	cs_highpass_set_freq(cself, NAN);
    }
    return freq;
}

void Init_highpass() {
    cCSHighpass = rb_define_class_under(mCSFilters, "Highpass", cCSFilter);

    rb_define_singleton_method(cCSHighpass, "new", rbcs_highpass_new, -1);
    rb_define_method(cCSHighpass, "freq", rbcs_highpass_freq, 0);
    rb_define_method(cCSHighpass, "freq=", rbcs_highpass_set_freq, 1);
}
