/*
 * rising_saw.c
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
#include <cshellsynth/rising_saw.h>
#include "jackruby.h"
#include "synths.h"
#include "synth.h"

static VALUE cCSRisingSaw;

static void cs_rsaw_free(void *mem) {
    cs_rsaw_t *cself = (cs_rsaw_t *) mem;
    cs_rsaw_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_rsaw_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "rsaw";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_rsaw_t *cself = ALLOC(cs_rsaw_t);
    int r = cs_rsaw_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_rsaw_free, cself);
}

void Init_rising_saw() {
    cCSRisingSaw = rb_define_class_under(mCSSynths, "LLRisingSaw", cCSSynth);

    rb_define_singleton_method(cCSRisingSaw, "new", rbcs_rsaw_new, -1);
}
