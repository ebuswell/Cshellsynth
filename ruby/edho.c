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
    cCSEdho = rb_define_class_under(mCSSynths, "Edho", cCSSynth);

    rb_define_singleton_method(cCSEdho, "new", rbcs_edho_new, -1);
    rb_define_method(cCSEdho, "bright", rbcs_edho_bright, 0);
    rb_define_method(cCSEdho, "bright=", rbcs_edho_set_bright, 1);
    rb_define_method(cCSEdho, "scale=", rbcs_edho_set_scale, 1);
}
