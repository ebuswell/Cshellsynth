#ifndef JACK_RUBY_H
#define JACK_RUBY_H 1

#include <bits/wordsize.h>
#include <ruby.h>
#include <jack/jack.h>

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

void jclient_free(void *mem);
void fake_free(void *mem);

#ifdef RUBY_THREADS

jack_port_t *j_client_port_register(jack_client_t *client, char *port_name, char *port_type, unsigned long flags, unsigned long buffer_size);
int j_client_port_unregister(jack_client_t *client, jack_port_t *port);
int j_mutex_lock(pthread_mutex_t *mutex);

#else

#define j_client_port_register(client, port_name, port_type, flags, buffer_size) \
    jack_port_register(client, port_name, port_type, flags, buffer_size)
#define j_mutex_lock(x) pthread_mutex_lock(x)
#define j_client_port_unregister(client, port) jack_port_unregister(client, port)

#endif // #ifdef RUBY_THREADS

#define j_mutex_unlock(x) pthread_mutex_unlock(x)
int j_mutex_init(pthread_mutex_t *mutex);

typedef struct jclient_struct {
    jack_client_t *client;
    int closed;
} jclient_t;

void jclient_init(char *name, jack_options_t flags, char *servername, jclient_t *cself);

#ifdef RUBY_THREAD

// FIXME: These functions block, need to create RUBY_THREADS versions

#else

#define j_client_close(client) jack_client_close(client)
#define j_client_activate(client) jack_activate(client)

#endif // #ifdef RUBY_THREAD

VALUE jclient_connect(VALUE self, VALUE source_port, VALUE destination_port);

#endif
