#include <ruby.h>
#include <cshellsynth/clock.h>
#include "jackruby.h"

static VALUE cCSClock;

static VALUE rbcs_clock_set_meter(VALUE self, VALUE meter) {
    cs_clock_t *cself;
    Data_Get_Struct(self, cs_clock_t, cself);
    cs_clock_set_meter(cself, NUM2DBL(meter));
    return meter;
}

static VALUE rbcs_clock_set_bpm(VALUE self, VALUE bpm) {
    cs_clock_t *cself;
    Data_Get_Struct(self, cs_clock_t, cself);
    cs_clock_set_bpm(cself, NUM2DBL(bpm));
    return bpm;
}

static VALUE rbcs_clock_clock(VALUE self) {
    VALUE clock_port = rb_iv_get(self, "@clock_port");
    if(NIL_P(clock_port)) {
	cs_clock_t *cself;
	Data_Get_Struct(self, cs_clock_t, cself);
	clock_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->clock_port);
	rb_iv_set(self, "@clock_port", clock_port);
    }
    return clock_port;
}

static void cs_clock_free(void *mem) {
    cs_clock_t *cself = (cs_clock_t *) mem;
    cs_clock_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_clock_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "clock";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_clock_t *cself = ALLOC(cs_clock_t);
    int r = cs_clock_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_clock_free, cself);
}

void Init_clock() {
    cCSClock = rb_define_class("Clock", cJackClient);

    rb_define_singleton_method(cCSClock, "new", rbcs_clock_new, -1);
    rb_define_method(cCSClock, "set_meter", rbcs_clock_set_meter, 1);
    rb_define_method(cCSClock, "set_bpm", rbcs_clock_set_bpm, 1);
    rb_define_method(cCSClock, "clock", rbcs_clock_clock, 0);
}
