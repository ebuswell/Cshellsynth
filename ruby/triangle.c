/*
 * triangle.c
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

static VALUE rbcs_triangle_set_slope(VALUE self, VALUE value) {
    cs_triangle_t *cself;
    Data_Get_Struct(self, cs_triangle_t, cself);
    cs_triangle_set_slope(cself, NUM2DBL(value));
    return value;
}

void Init_triangle() {
    cCSTriangle = rb_define_class_under(mCSSynths, "LLTriangle", cCSSynth);

    rb_define_singleton_method(cCSTriangle, "new", rbcs_triangle_new, -1);
    rb_define_method(cCSTriangle, "slope=", rbcs_triangle_set_slope, 1);
}
