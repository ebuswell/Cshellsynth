#include <ruby.h>
#include <cshellsynth/square.h>
#include "jackruby.h"
#include "synths.h"
#include "synth.h"

static VALUE cCSSquare;

static void cs_square_free(void *mem) {
    cs_square_t *cself = (cs_square_t *) mem;
    cs_square_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_square_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "square";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_square_t *cself = ALLOC(cs_square_t);
    int r = cs_square_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_square_free, cself);
}

void Init_square() {
    cCSSquare = rb_define_class_under(mCSSynths, "Square", cCSSynth);

    rb_define_singleton_method(cCSSquare, "new", rbcs_square_new, -1);
}
