#include <ruby.h>
#include "controllers.h"

VALUE mCSControllers;

void Init_controllers() {
    mCSControllers = rb_define_module("Controllers");	
}
