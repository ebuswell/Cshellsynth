#include <ruby.h>
#include <cshellsynth/distortion.h>
#include "jackruby.h"
#include "filters.h"
#include "filter.h"

static VALUE cCSDistortion;

static void cs_distort_free(void *mem) {
    cs_distort_t *cself = (cs_distort_t *) mem;
    cs_distort_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_distort_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "distort";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_distort_t *cself = ALLOC(cs_distort_t);
    int r = cs_distort_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_distort_free, cself);
}

void Init_distortion() {
    cCSDistortion = rb_define_class_under(mCSFilters, "Distortion", cCSFilter);

    rb_define_singleton_method(cCSDistortion, "new", rbcs_distort_new, -1);
}
