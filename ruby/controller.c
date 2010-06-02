#include <ruby.h>
#include <cshellsynth/controller.h>
#include "jackruby.h"
#include "controllers.h"
#include "controller.h"

VALUE cCSController;

static VALUE rbcs_ctlr_out(VALUE self) {
    VALUE out_port = rb_iv_get(self, "@out_port");
    if(NIL_P(out_port)) {
	cs_ctlr_t *cself;
	Data_Get_Struct(self, cs_ctlr_t, cself);
	out_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->out_port);
	rb_iv_set(self, "@out_port", out_port);
    }
    return out_port;
}

static VALUE rbcs_ctlr_ctl(VALUE self) {
    VALUE ctl_port = rb_iv_get(self, "@ctl_port");
    if(NIL_P(ctl_port)) {
	cs_ctlr_t *cself;
	Data_Get_Struct(self, cs_ctlr_t, cself);
	ctl_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->ctl_port);
	rb_iv_set(self, "@ctl_port", ctl_port);
    }
    return ctl_port;
}

static void cs_ctlr_free(void *mem) {
    cs_ctlr_t *cself = (cs_ctlr_t *) mem;
    cs_ctlr_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_ctlr_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname, rflags, rservername;
    rb_scan_args(argc, argv, "21", &rname, &rflags, &rservername);
    char *name = StringValueCStr(rname);
    jack_options_t flags = NUM2JACKOPTIONST(rflags);
    char *servername = NIL_P(rservername) ? NULL : StringValueCStr(rservername);
    cs_ctlr_t *cself = ALLOC(cs_ctlr_t);
    int r = cs_ctlr_init(cself, name, flags, servername);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_ctlr_free, cself);
}

void Init_controller() {
    cCSController = rb_define_class_under(mCSControllers, "Controller", cJackClient);

    rb_define_singleton_method(cCSController, "new", rbcs_ctlr_new, -1);
    rb_define_method(cCSController, "out", rbcs_ctlr_out, 0);
    rb_define_method(cCSController, "ctl", rbcs_ctlr_ctl, 0);
}
