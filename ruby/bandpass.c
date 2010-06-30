#include <ruby.h>
#include <cshellsynth/bandpass.h>
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
    if(KIND_OF(freq, rb_cNumeric)) {
	cs_bandpass_t *cself;
	Data_Get_Struct(self, cs_bandpass_t, cself);
	cs_bandpass_set_freq(cself, NUM2DBL(freq));
    } else {
	VALUE freq_port = rb_iv_get(self, "@freq_port");
	if(NIL_P(freq_port)) {
	    cs_bandpass_t *cself;
	    Data_Get_Struct(self, cs_bandpass_t, cself);
	    freq_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->freq_port);
	    rb_iv_set(self, "@freq_port", freq_port);
	}
	jr_client_connect(self, freq, freq_port);
	// ignore return value
    }
    return freq;
}

void Init_bandpass() {
    cCSBandpass = rb_define_class_under(mCSFilters, "Bandpass", cCSFilter);

    rb_define_singleton_method(cCSBandpass, "new", rbcs_bandpass_new, -1);
    rb_define_method(cCSBandpass, "freq", rbcs_bandpass_freq, 0);
    rb_define_method(cCSBandpass, "freq=", rbcs_bandpass_set_freq, 1);
}
