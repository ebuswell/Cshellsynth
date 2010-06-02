#include <ruby.h>
#include "synths.h"

VALUE mCSSynths;

void Init_synths() {
    mCSSynths = rb_define_module("Synths");	
}
