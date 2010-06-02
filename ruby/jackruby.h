#ifndef JACK_RUBY_H
#define JACK_RUBY_H 1

#include <bits/wordsize.h>
#include <ruby.h>
#include <jack/jack.h>
#include <cshellsynth/jclient.h>

#ifndef rb_str_new2 // if it's not a definition, not if it doesn't exist
#define rb_str_new_cstr rb_str_new2
#endif

#ifdef RUBY_UBF_IO
#define RUBY_THREADS 1
#endif

#ifdef RUBY_THREADS
#define JACK_RUBY_UBF RUBY_UBF_IO
#endif

VALUE mJack;
VALUE cJackClient, cJackPort;
VALUE eJackServerFailed, eJackServerError, eJackNoSuchClient, eJackLoadFailure, eJackInitFailure,
    eJackShmFailure, eJackVersionError, eJackBackendError, eJackClientZombie, eJackInvalidOption,
    eJackNameNotUnique, eJackFailure;

#ifndef DBL2NUM
#define DBL2NUM(dbl) rb_float_new(dbl)
#endif

#ifndef NUM2SIZET
#define NUM2SIZET(x) NUM2ULONG(x)
#endif

#define INT32T2NUM(x) INT2NUM(x)
#define UINT32T2NUM(x) UINT2NUM(x)
#define NUM2INT32T(x) NUM2INT(x)
#define NUM2UINT32T(x) NUM2UINT(x)
#if __WORDSIZE == 64
# define UINT64T2NUM(x) ULONG2NUM(x)
# define NUM2UINT64T(x) NUM2ULONG(x)
#else
# define UINT64T2NUM(x) ULL2NUM(x)
# define NUM2UINT64T(x) NUM2ULL(x)
#endif

#define JACKNFRAMEST2NUM(x) UINT32T2NUM(x)
#define NUM2JACKNFRAMEST(x) NUM2UINT32T(x)
#define JACKSTATUST2NUM(x) INT2NUM(x)
#define JACKPORTIDT2NUM(x) UINT32T2NUM(x)
#define NUM2JACKOPTIONST(x) NUM2INT(x)
#define NUM2JACKPORTIDT(x) NUM2UINT32T(x)
#define NUM2JACKTIMET(x) NUM2UINT64T(x)

#define IS_TRUE(t) is_true(t)

inline int is_true(VALUE t);

#define KIND_OF(v,klass) (rb_obj_is_kind_of(v,klass) == Qtrue)

void fake_free(void *mem);
VALUE jr_client_connect(VALUE self, VALUE rsource_port, VALUE rdestination_port);

#define JR_CHECK_INIT_ERROR(cself, r)					\
    do {								\
    if(r != 0) {							\
	xfree(cself);							\
	if(r & JackServerFailed) {					\
	    rb_raise(eJackServerFailed, "Unable to connect to the JACK server"); \
	} else if(r & JackServerError) {				\
	    rb_raise(eJackServerError, "Communication error with the JACK server"); \
	} else if(r & JackNoSuchClient) {				\
	    rb_raise(eJackNoSuchClient, "Requested client does not exist"); \
	} else if(r & JackLoadFailure) {				\
	    rb_raise(eJackLoadFailure, "Unable to load internal client"); \
	} else if(r & JackInitFailure) {				\
	    rb_raise(eJackInitFailure, "Unable to initialize client");	\
	} else if(r & JackShmFailure) {					\
	    rb_raise(eJackShmFailure, "Unable to access shared memory"); \
	} else if(r & JackVersionError) {				\
	    rb_raise(eJackVersionError, "Client's protocol version does not match"); \
	} else if(r & JackBackendError) {				\
	    rb_raise(eJackBackendError, "Backend error");		\
	} else if(r & JackClientZombie) {				\
	    rb_raise(eJackClientZombie, "Client zombie");		\
	} else if(r & JackInvalidOption) {				\
	    rb_raise(eJackInvalidOption, "The operation contained an invalid or unsupported option"); \
	} else if(r & JackNameNotUnique) {				\
	    rb_raise(eJackNameNotUnique, "The desired client name was not unique"); \
	} else {							\
	    rb_raise(eJackFailure, "Overall operation failed: %d", r);	\
	}								\
    }									\
    } while(0)


#endif
