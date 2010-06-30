#include <ruby.h>
#include <cshellsynth/lin2exp.h>
#include "jackruby.h"
#include "filters.h"
#include "filter.h"

static VALUE cCSLin2Exp;

static void cs_lin2exp_free(void *mem) {
    cs_lin2exp_t *cself = (cs_lin2exp_t *) mem;
    cs_lin2exp_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_lin2exp_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "lin2exp";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_lin2exp_t *cself = ALLOC(cs_lin2exp_t);
    int r = cs_lin2exp_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_lin2exp_free, cself);
}

void Init_lin2exp() {
    cCSLin2Exp = rb_define_class_under(mCSFilters, "Lin2Exp", cCSFilter);

    rb_define_singleton_method(cCSLin2Exp, "new", rbcs_lin2exp_new, -1);
}
