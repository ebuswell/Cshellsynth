/*
 * square.c
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

static VALUE rbcs_square_set_duty_cycle(VALUE self, VALUE value) {
    cs_square_t *cself;
    Data_Get_Struct(self, cs_square_t, cself);
    cs_square_set_duty_cycle(cself, NUM2DBL(value));
    return value;
}

void Init_square() {
    cCSSquare = rb_define_class_under(mCSSynths, "LLSquare", cCSSynth);

    rb_define_singleton_method(cCSSquare, "new", rbcs_square_new, -1);
    rb_define_method(cCSSquare, "duty_cycle=", rbcs_square_set_duty_cycle, 1);
}
