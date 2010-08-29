/*
 * impulse_train.c
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
#include <cshellsynth/impulse_train.h>
#include <math.h>
#include "jackruby.h"
#include "synths.h"
#include "synth.h"

static VALUE cCSImpulseTrain;

static void cs_itrain_free(void *mem) {
    cs_itrain_t *cself = (cs_itrain_t *) mem;
    cs_itrain_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_itrain_set_scale(VALUE self, VALUE scale) {
    cs_itrain_t *cself;
    Data_Get_Struct(self, cs_itrain_t, cself);
    cs_itrain_set_scale(cself, IS_TRUE(scale));
    return scale;
}

static VALUE rbcs_itrain_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "itrain";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_itrain_t *cself = ALLOC(cs_itrain_t);
    int r = cs_itrain_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_itrain_free, cself);
}

void Init_impulse_train() {
    cCSImpulseTrain = rb_define_class_under(mCSSynths, "LLImpulseTrain", cCSSynth);

    rb_define_singleton_method(cCSImpulseTrain, "new", rbcs_itrain_new, -1);
    rb_define_method(cCSImpulseTrain, "scale=", rbcs_itrain_set_scale, 1);
}
