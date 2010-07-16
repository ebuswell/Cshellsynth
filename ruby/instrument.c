/*
 * instrument.c
 * 
 * Copyright 2010 Evan Buswell
 * 
 * This file is part of Cshellsynth.
 * 
 * Cshellsynth is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Cshellsynth is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Cshellsynth.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <ruby.h>
#include <cshellsynth/instrument.h>
#include "jackruby.h"
#include "controllers.h"
#include "controller.h"

static VALUE cCSInstrument;

static VALUE rbcs_inst_play(VALUE self, VALUE value) {
    cs_inst_t *cself;
    Data_Get_Struct(self, cs_inst_t, cself);
    cs_inst_play(cself, NUM2DBL(value));
    return self;
}

static VALUE rbcs_inst_stop(VALUE self) {
    cs_inst_t *cself;
    Data_Get_Struct(self, cs_inst_t, cself);
    cs_inst_stop(cself);
    return self;
}

static void cs_inst_free(void *mem) {
    cs_inst_t *cself = (cs_inst_t *) mem;
    cs_inst_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_inst_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "inst";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_inst_t *cself = ALLOC(cs_inst_t);
    int r = cs_inst_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_inst_free, cself);
}

void Init_instrument() {
    cCSInstrument = rb_define_class_under(mCSControllers, "Instrument", cCSController);

    rb_define_singleton_method(cCSInstrument, "new", rbcs_inst_new, -1);
    rb_define_method(cCSInstrument, "play", rbcs_inst_play, 1);
    rb_define_method(cCSInstrument, "stop", rbcs_inst_stop, 0);
}
