#include <ruby.h>
#include <cshellsynth/infh.h>
#include "jackruby.h"
#include "synths.h"
#include "synth.h"

static VALUE cCSInfH;

static void cs_infh_free(void *mem) {
    cs_infh_t *cself = (cs_infh_t *) mem;
    cs_infh_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_infh_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "infh";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_infh_t *cself = ALLOC(cs_infh_t);
    int r = cs_infh_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_infh_free, cself);
}

void Init_infh() {
    cCSInfH = rb_define_class_under(mCSSynths, "InfH", cCSSynth);

    rb_define_singleton_method(cCSInfH, "new", rbcs_infh_new, -1);
}
