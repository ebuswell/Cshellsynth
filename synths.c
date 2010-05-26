#include "synths.h"
#include "jackruby.h"

void Init_synths() {
    mJSynths = rb_define_module_under(mJack, "Synths");	
}
