#include <ruby.h>
#include <cshellsynth/key.h>
#include <math.h>
#include "jackruby.h"

static VALUE cCSKey;

static VALUE rbcs_key_major_tuning;
static VALUE rbcs_key_minor_tuning;
static VALUE rbcs_key_pythagorean_tuning;
static VALUE rbcs_key_equal_tuning;

static VALUE rbcs_key_note2freq(VALUE self, VALUE value) {
    cs_key_t *cself;
    Data_Get_Struct(self, cs_key_t, cself);
    return DBL2NUM(cs_key_note2freq(cself, NUM2DBL(value)));
}

static VALUE rbcs_key_set_tuning(VALUE self, VALUE value) {
    cs_key_t *cself;
    Data_Get_Struct(self, cs_key_t, cself);
    int r;
    if(value == rbcs_key_major_tuning) {
	r = cs_key_set_tuning(cself, CS_MAJOR_TUNING, CS_MAJOR_TUNING_LENGTH);
    } else if(value == rbcs_key_minor_tuning) {
	r = cs_key_set_tuning(cself, CS_MINOR_TUNING, CS_MINOR_TUNING_LENGTH);
    } else if(value == rbcs_key_pythagorean_tuning) {
	r = cs_key_set_tuning(cself, CS_PYTHAGOREAN_TUNING, CS_PYTHAGOREAN_TUNING_LENGTH);
    } else if(value == rbcs_key_equal_tuning) {
	r = cs_key_set_tuning(cself, CS_EQUAL_TUNING, CS_EQUAL_TUNING_LENGTH);
    } else {
	if(TYPE(value) != T_ARRAY) {
	    rb_raise(rb_eArgError, "tuning must be an array");
	}
	size_t tuning_length = RARRAY_LEN(value);
	double *tuning = ALLOCA_N(double, tuning_length);
	int i;
	for(i = 0; i < tuning_length; i++) {
	    tuning[i] = NUM2DBL(RARRAY_PTR(value)[i]);
	}
	r = cs_key_set_tuning(cself, tuning, tuning_length);
    }
    if(r != 0) {
	rb_raise(eJackFailure, "Overall operation failed: %d", r);
    }
    return value;
}

static VALUE rbcs_key_note(VALUE self) {
    cs_key_t *cself;
    Data_Get_Struct(self, cs_key_t, cself);
    VALUE note_port = rb_iv_get(self, "@note_port");
    if(NIL_P(note_port)) {
	note_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->note_port);
	rb_iv_set(self, "@note_port", note_port);
    }
    return note_port;
}

static VALUE rbcs_key_set_note(VALUE self, VALUE note) {
    if(KIND_OF(note, rb_cNumeric)) {
	cs_key_t *cself;
	Data_Get_Struct(self, cs_key_t, cself);
	cs_key_set_note(cself, NUM2DBL(note));
    } else {
	VALUE note_port = rb_iv_get(self, "@note_port");
	if(NIL_P(note_port)) {
	    cs_key_t *cself;
	    Data_Get_Struct(self, cs_key_t, cself);
	    note_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->note_port);
	    rb_iv_set(self, "@note_port", note_port);
	}
	jr_client_connect(self, note, note_port);
	// ignore return value
    }
    return note;
}

static VALUE rbcs_key_root(VALUE self) {
    cs_key_t *cself;
    Data_Get_Struct(self, cs_key_t, cself);
    VALUE root_port = rb_iv_get(self, "@root_port");
    if(NIL_P(root_port)) {
	root_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->root_port);
	rb_iv_set(self, "@root_port", root_port);
    }
    return root_port;
}

static VALUE rbcs_key_set_root(VALUE self, VALUE root) {
    if(KIND_OF(root, rb_cNumeric)) {
	cs_key_t *cself;
	Data_Get_Struct(self, cs_key_t, cself);
	cs_key_set_root(cself, NUM2DBL(root));
    } else {
	VALUE root_port = rb_iv_get(self, "@root_port");
	if(NIL_P(root_port)) {
	    cs_key_t *cself;
	    Data_Get_Struct(self, cs_key_t, cself);
	    root_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->root_port);
	    rb_iv_set(self, "@root_port", root_port);
	}
	jr_client_connect(self, root, root_port);
	// ignore return value
    }
    return root;
}

static VALUE rbcs_key_freq(VALUE self) {
    VALUE freq_port = rb_iv_get(self, "@freq_port");
    if(NIL_P(freq_port)) {
	cs_key_t *cself;
	Data_Get_Struct(self, cs_key_t, cself);
	freq_port = Data_Wrap_Struct(cJackPort, 0, fake_free, cself->freq_port);
	rb_iv_set(self, "@freq_port", freq_port);
    }
    return freq_port;
}

static void cs_key_free(void *mem) {
    cs_key_t *cself = (cs_key_t *) mem;
    cs_key_destroy(cself);
    xfree(cself);
}

static VALUE rbcs_key_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname;
    char *name = "key";
    if(rb_scan_args(argc, argv, "01", &rname)) {
	name = StringValueCStr(rname);
    }
    cs_key_t *cself = ALLOC(cs_key_t);
    int r = cs_key_init(cself, name, 0, NULL);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, cs_key_free, cself);
}

void Init_key() {
    cCSKey = rb_define_class("Key", cJackClient);

    rb_define_singleton_method(cCSKey, "new", rbcs_key_new, -1);
    rb_define_method(cCSKey, "note", rbcs_key_note, 0);
    rb_define_method(cCSKey, "note=", rbcs_key_set_note, 1);
    rb_define_method(cCSKey, "note2freq", rbcs_key_note2freq, 1);
    rb_define_method(cCSKey, "freq", rbcs_key_freq, 0);
    rb_define_method(cCSKey, "root", rbcs_key_root, 0);
    rb_define_method(cCSKey, "root=", rbcs_key_set_root, 1);
    rb_define_method(cCSKey, "tuning=", rbcs_key_set_tuning, 1);

    rbcs_key_major_tuning = rb_ary_new2(CS_MAJOR_TUNING_LENGTH);
    int i;
    for(i = 0; i < CS_MAJOR_TUNING_LENGTH; i++) {
	rb_ary_push(rbcs_key_major_tuning, DBL2NUM(CS_MAJOR_TUNING[i]));
    }
    rb_define_const(cCSKey, "Major", rbcs_key_major_tuning);

    rbcs_key_minor_tuning = rb_ary_new2(CS_MINOR_TUNING_LENGTH);
    for(i = 0; i < CS_MINOR_TUNING_LENGTH; i++) {
	rb_ary_push(rbcs_key_minor_tuning, DBL2NUM(CS_MINOR_TUNING[i]));
    }
    rb_define_const(cCSKey, "Minor", rbcs_key_minor_tuning);

    rbcs_key_pythagorean_tuning = rb_ary_new2(CS_PYTHAGOREAN_TUNING_LENGTH);
    for(i = 0; i < CS_PYTHAGOREAN_TUNING_LENGTH; i++) {
	rb_ary_push(rbcs_key_pythagorean_tuning, DBL2NUM(CS_PYTHAGOREAN_TUNING[i]));
    }
    rb_define_const(cCSKey, "Pythagorean", rbcs_key_pythagorean_tuning);

    rbcs_key_equal_tuning = rb_ary_new2(CS_EQUAL_TUNING_LENGTH);
    rb_define_const(cCSKey, "Equal", rbcs_key_equal_tuning);
    for(i = 0; i < CS_EQUAL_TUNING_LENGTH; i++) {
	rb_ary_push(rbcs_key_equal_tuning, DBL2NUM(pow(2.0, ((double) i)/12.0)));
    }

    rb_define_const(cCSKey, "A", DBL2NUM(CS_A));
    rb_define_const(cCSKey, "A_Sharp", DBL2NUM(CS_A_SHARP));
    rb_define_const(cCSKey, "B_Flat", DBL2NUM(CS_B_FLAT));
    rb_define_const(cCSKey, "B", DBL2NUM(CS_B));
    rb_define_const(cCSKey, "C_Flat", DBL2NUM(CS_C_FLAT));
    rb_define_const(cCSKey, "C", DBL2NUM(CS_C));
    rb_define_const(cCSKey, "B_Sharp", DBL2NUM(CS_B_SHARP));
    rb_define_const(cCSKey, "C_Sharp", DBL2NUM(CS_C_SHARP));
    rb_define_const(cCSKey, "D_Flat", DBL2NUM(CS_D_FLAT));
    rb_define_const(cCSKey, "D", DBL2NUM(CS_D));
    rb_define_const(cCSKey, "D_Sharp", DBL2NUM(CS_D_SHARP));
    rb_define_const(cCSKey, "E_Flat", DBL2NUM(CS_E_FLAT));
    rb_define_const(cCSKey, "E", DBL2NUM(CS_E));
    rb_define_const(cCSKey, "F_Flat", DBL2NUM(CS_F_FLAT));
    rb_define_const(cCSKey, "F", DBL2NUM(CS_F));
    rb_define_const(cCSKey, "E_Sharp", DBL2NUM(CS_E_SHARP));
    rb_define_const(cCSKey, "F_Sharp", DBL2NUM(CS_F_SHARP));
    rb_define_const(cCSKey, "G_Flat", DBL2NUM(CS_G_FLAT));
    rb_define_const(cCSKey, "G", DBL2NUM(CS_G));
    rb_define_const(cCSKey, "G_Sharp", DBL2NUM(CS_G_SHARP));
    rb_define_const(cCSKey, "A_Flat", DBL2NUM(CS_A_FLAT));
}
