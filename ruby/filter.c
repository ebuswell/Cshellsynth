#include <ruby.h>
#include <math.h>
#include <cshellsynth/filter.h>
#include "jackruby.h"
#include "filters.h"
#include "filter.h"

VALUE cCSFilter;

static VALUE rbcs_filter_in(VALUE self) {
    cs_filter_t *cself;
    Data_Get_Struct(self, cs_filter_t, cself);
    VALUE in_port = rb_iv_get(self, "@in_port");
    if(NIL_P(in_port)) {
	in_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->in_port);
	rb_iv_set(self, "@in_port", in_port);
    }
    return in_port;
}

static VALUE rbcs_filter_set_in(VALUE self, VALUE in) {
    cs_filter_t *cself;
    Data_Get_Struct(self, cs_filter_t, cself);
    if(KIND_OF(in, rb_cNumeric)) {
	cs_filter_set_in(cself, NUM2DBL(in));
    } else {
	VALUE in_port = rb_iv_get(self, "@in_port");
	if(NIL_P(in_port)) {
	    in_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->in_port);
	    rb_iv_set(self, "@in_port", in_port);
	}
	jr_client_connect(self, in, in_port);
	// ignore return value
	cs_filter_set_in(cself, NAN);
    }
    return in;
}

static VALUE rbcs_filter_out(VALUE self) {
    VALUE out_port = rb_iv_get(self, "@out_port");
    if(NIL_P(out_port)) {
	cs_filter_t *cself;
	Data_Get_Struct(self, cs_filter_t, cself);
	out_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->out_port);
	rb_iv_set(self, "@out_port", out_port);
    }
    return out_port;
}

static void cs_filter_free(void *mem) {
    cs_filter_t *cself = (cs_filter_t *) mem;
    cs_filter_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_filter_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname, rflags, rservername;
    rb_scan_args(argc, argv, "21", &rname, &rflags, &rservername);
    char *name = StringValueCStr(rname);
    jack_options_t flags = NUM2JACKOPTIONST(rflags);
    char *servername = NIL_P(rservername) ? NULL : StringValueCStr(rservername);
    cs_filter_t *cself = ALLOC(cs_filter_t);
    int r = cs_filter_init(cself, name, flags, servername);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_filter_free, cself);
}

void Init_filter() {
    cCSFilter = rb_define_class_under(mCSFilters, "Filter", cJackClient);

    rb_define_singleton_method(cCSFilter, "new", rbcs_filter_new, -1);
    rb_define_method(cCSFilter, "in", rbcs_filter_in, 0);
    rb_define_method(cCSFilter, "in=", rbcs_filter_set_in, 1);
    rb_define_method(cCSFilter, "out", rbcs_filter_out, 0);
}
