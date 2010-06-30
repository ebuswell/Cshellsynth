#include <ruby.h>
#include "filters.h"

VALUE mCSFilters;

void Init_filters() {
    mCSFilters = rb_define_module("Filters");	
}
