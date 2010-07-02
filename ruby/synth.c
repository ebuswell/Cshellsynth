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
}
