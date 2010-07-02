#include <ruby.h>
#include <math.h>
#include <cshellsynth/vocalizer.h>
#include "jackruby.h"

static VALUE cCSVocalizer;

static VALUE rbcs_vocalizer_ctl(VALUE self) {
    cs_vocalizer_t *cself;
    Data_Get_Struct(self, cs_vocalizer_t, cself);
    VALUE ctl_port = rb_iv_get(self, "@ctl_port");
    if(NIL_P(ctl_port)) {
	ctl_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->ctl_port);
	rb_iv_set(self, "@ctl_port", ctl_port);
    }
    return ctl_port;
}

static VALUE rbcs_vocalizer_set_ctl(VALUE self, VALUE ctl) {
    VALUE ctl_port = rb_iv_get(self, "@ctl_port");
    if(NIL_P(ctl_port)) {
	cs_vocalizer_t *cself;
	Data_Get_Struct(self, cs_vocalizer_t, cself);
	ctl_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->ctl_port);
	rb_iv_set(self, "@ctl_port", ctl_port);
    }
    jr_client_connect(self, ctl, ctl_port);
    // ignore return value
    return ctl;
}

static VALUE rbcs_vocalizer_load(VALUE self, VALUE path) {
    cs_vocalizer_t *cself;
    Data_Get_Struct(self, cs_vocalizer_t, cself);
    int r = cs_vocalizer_load(cself, StringValueCStr(path));
    if(r != 0) {
	rb_raise(eJackFailure, "Overall operation failed: %d", r);
    }
    return path;
}

static VALUE rbcs_vocalizer_out(VALUE self) {
    VALUE out_port = rb_iv_get(self, "@out_port");
    if(NIL_P(out_port)) {
	cs_vocalizer_t *cself;
	Data_Get_Struct(self, cs_vocalizer_t, cself);
	out_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->out_port);
	rb_iv_set(self, "@out_port", out_port);
    }
    return out_port;
}

static void cs_vocalizer_free(void *mem) {
    cs_vocalizer_t *cself = (cs_vocalizer_t *) mem;
    cs_vocalizer_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_vocalizer_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "vocalizer";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_vocalizer_t *cself = ALLOC(cs_vocalizer_t);
    int r = cs_vocalizer_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_vocalizer_free, cself);
}

void Init_vocalizer() {
    cCSVocalizer = rb_define_class("Vocalizer", cJackClient);

    rb_define_singleton_method(cCSVocalizer, "new", rbcs_vocalizer_new, -1);
    rb_define_method(cCSVocalizer, "load", rbcs_vocalizer_load, 1);
    rb_define_method(cCSVocalizer, "ctl", rbcs_vocalizer_ctl, 0);
    rb_define_method(cCSVocalizer, "ctl=", rbcs_vocalizer_set_ctl, 1);
    rb_define_method(cCSVocalizer, "out", rbcs_vocalizer_out, 0);
}
