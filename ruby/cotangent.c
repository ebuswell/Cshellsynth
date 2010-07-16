/*
 * cotangent.c
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
#include <cshellsynth/cot.h>
#include "jackruby.h"
#include "synths.h"
#include "synth.h"

static VALUE cCSCotangent;

static void cs_cot_free(void *mem) {
    cs_cot_t *cself = (cs_cot_t *) mem;
    cs_cot_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_cot_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "cot";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_cot_t *cself = ALLOC(cs_cot_t);
    int r = cs_cot_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_cot_free, cself);
}

void Init_cotangent() {
    cCSCotangent = rb_define_class_under(mCSSynths, "Cotangent", cCSSynth);

    rb_define_singleton_method(cCSCotangent, "new", rbcs_cot_new, -1);
}
