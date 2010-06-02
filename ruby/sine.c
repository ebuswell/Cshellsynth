#include <ruby.h>
#include <cshellsynth/sine.h>
#include "jackruby.h"
#include "synths.h"
#include "synth.h"

static VALUE cCSSine;

static void cs_sine_free(void *mem) {
    cs_sine_t *cself = (cs_sine_t *) mem;
    cs_sine_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_sine_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "sine";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_sine_t *cself = ALLOC(cs_sine_t);
    int r = cs_sine_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_sine_free, cself);
}

void Init_sine() {
    cCSSine = rb_define_class_under(mCSSynths, "Sine", cCSSynth);

    rb_define_singleton_method(cCSSine, "new", rbcs_sine_new, -1);
}
