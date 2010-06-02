#include <ruby.h>
#include <cshellsynth/falling_saw.h>
#include "jackruby.h"
#include "synths.h"
#include "synth.h"

static VALUE cCSFallingSaw;

static void cs_fsaw_free(void *mem) {
    cs_fsaw_t *cself = (cs_fsaw_t *) mem;
    cs_fsaw_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_fsaw_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "fsaw";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_fsaw_t *cself = ALLOC(cs_fsaw_t);
    int r = cs_fsaw_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_fsaw_free, cself);
}

void Init_falling_saw() {
    cCSFallingSaw = rb_define_class_under(mCSSynths, "FallingSaw", cCSSynth);

    rb_define_singleton_method(cCSFallingSaw, "new", rbcs_fsaw_new, -1);
}
