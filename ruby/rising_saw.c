#include <ruby.h>
#include <cshellsynth/rising_saw.h>
#include "jackruby.h"
#include "synths.h"
#include "synth.h"

static VALUE cCSRisingSaw;

static void cs_rsaw_free(void *mem) {
    cs_rsaw_t *cself = (cs_rsaw_t *) mem;
    cs_rsaw_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_rsaw_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "rsaw";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_rsaw_t *cself = ALLOC(cs_rsaw_t);
    int r = cs_rsaw_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_rsaw_free, cself);
}

void Init_rising_saw() {
    cCSRisingSaw = rb_define_class_under(mCSSynths, "RisingSaw", cCSSynth);

    rb_define_singleton_method(cCSRisingSaw, "new", rbcs_rsaw_new, -1);
}
