/*
 * notch.c
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
#include <cshellsynth/notch.h>
#include "jackruby.h"
#include "filters.h"
#include "lowpass.h"

static VALUE cCSNotch;

static void cs_notch_free(void *mem) {
    cs_notch_t *cself = (cs_notch_t *) mem;
    cs_notch_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_notch_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "notch";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_notch_t *cself = ALLOC(cs_notch_t);
    int r = cs_notch_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_notch_free, cself);
}

void Init_notch() {
    cCSNotch = rb_define_class_under(mCSFilters, "LLNotch", cCSLowpass);

    rb_define_singleton_method(cCSNotch, "new", rbcs_notch_new, -1);
}
