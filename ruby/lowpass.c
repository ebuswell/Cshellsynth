#include <ruby.h>
#include <cshellsynth/lowpass.h>
#include <math.h> /* for NAN */
#include "jackruby.h"
#include "filters.h"
#include "filter.h"

static VALUE cCSLowpass;

static void cs_lowpass_free(void *mem) {
    cs_lowpass_t *cself = (cs_lowpass_t *) mem;
    cs_lowpass_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_lowpass_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "lowpass";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_lowpass_t *cself = ALLOC(cs_lowpass_t);
    int r = cs_lowpass_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_lowpass_free, cself);
}

static VALUE rbcs_lowpass_freq(VALUE self) {
    cs_lowpass_t *cself;
    Data_Get_Struct(self, cs_lowpass_t, cself);
    VALUE freq_port = rb_iv_get(self, "@freq_port");
    if(NIL_P(freq_port)) {
	freq_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->freq_port);
	rb_iv_set(self, "@freq_port", freq_port);
    }
    return freq_port;
}

static VALUE rbcs_lowpass_set_freq(VALUE self, VALUE freq) {
    cs_lowpass_t *cself;
    Data_Get_Struct(self, cs_lowpass_t, cself);
    if(KIND_OF(freq, rb_cNumeric)) {
	cs_lowpass_set_freq(cself, NUM2DBL(freq));
    } else {
	VALUE freq_port = rb_iv_get(self, "@freq_port");
	if(NIL_P(freq_port)) {
	    freq_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->freq_port);
	    rb_iv_set(self, "@freq_port", freq_port);
	}
	jr_client_connect(self, freq, freq_port);
	// ignore return value
	cs_lowpass_set_freq(cself, NAN);
    }
    return freq;
}

void Init_lowpass() {
    cCSLowpass = rb_define_class_under(mCSFilters, "Lowpass", cCSFilter);

    rb_define_singleton_method(cCSLowpass, "new", rbcs_lowpass_new, -1);
    rb_define_method(cCSLowpass, "freq", rbcs_lowpass_freq, 0);
    rb_define_method(cCSLowpass, "freq=", rbcs_lowpass_set_freq, 1);
}
