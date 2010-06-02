#include <ruby.h>
#include <cshellsynth/modulator.h>
#include "mixer.h"
#include "jackruby.h"

static VALUE cCSModulator;

static void cs_modu_free(void *mem) {
    cs_modu_t *cself = (cs_modu_t *) mem;
    cs_modu_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_modu_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "modu";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_modu_t *cself = ALLOC(cs_modu_t);
    int r = cs_modu_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_modu_free, cself);
}

void Init_modulator() {
    cCSModulator = rb_define_class("Modulator", cCSMixer);

    rb_define_singleton_method(cCSModulator, "new", rbcs_modu_new, -1);
}
