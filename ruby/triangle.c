#include <ruby.h>
#include <cshellsynth/triangle.h>
#include "jackruby.h"
#include "synths.h"
#include "synth.h"

static VALUE cCSTriangle;

static void cs_triangle_free(void *mem) {
    cs_triangle_t *cself = (cs_triangle_t *) mem;
    cs_triangle_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_triangle_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "triangle";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_triangle_t *cself = ALLOC(cs_triangle_t);
    int r = cs_triangle_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_triangle_free, cself);
}

void Init_triangle() {
    cCSTriangle = rb_define_class_under(mCSSynths, "Triangle", cCSSynth);

    rb_define_singleton_method(cCSTriangle, "new", rbcs_triangle_new, -1);
}
