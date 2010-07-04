#include <ruby.h>
#include <math.h>
#include <cshellsynth/portamento.h>
#include "jackruby.h"
#include "filters.h"
#include "filter.h"

static VALUE cCSPortamento;

static void cs_porta_free(void *mem) {
    cs_porta_t *cself = (cs_porta_t *) mem;
    cs_porta_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_porta_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "porta";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_porta_t *cself = ALLOC(cs_porta_t);
    int r = cs_porta_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_porta_free, cself);
}

static VALUE rbcs_porta_lag(VALUE self) {
    cs_porta_t *cself;
    Data_Get_Struct(self, cs_porta_t, cself);
    VALUE lag_port = rb_iv_get(self, "@lag_port");
    if(NIL_P(lag_port)) {
	lag_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->lag_port);
	rb_iv_set(self, "@lag_port", lag_port);
    }
    return lag_port;
}

static VALUE rbcs_porta_set_lag(VALUE self, VALUE lag) {
    cs_porta_t *cself;
    Data_Get_Struct(self, cs_porta_t, cself);
    if(KIND_OF(lag, rb_cNumeric)) {
	cs_porta_set_lag(cself, NUM2DBL(lag));
    } else {
	VALUE lag_port = rb_iv_get(self, "@lag_port");
	if(NIL_P(lag_port)) {
	    lag_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->lag_port);
	    rb_iv_set(self, "@lag_port", lag_port);
	}
	jr_client_connect(self, lag, lag_port);
	// ignore return value
	cs_porta_set_lag(cself, NAN);
    }
    return lag;
}

void Init_portamento() {
    cCSPortamento = rb_define_class_under(mCSFilters, "Portamento", cCSFilter);

    rb_define_singleton_method(cCSPortamento, "new", rbcs_porta_new, -1);
    rb_define_method(cCSPortamento, "lag", rbcs_porta_lag, 0);
    rb_define_method(cCSPortamento, "lag=", rbcs_porta_set_lag, 1);
}
