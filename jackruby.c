#include <ruby.h>
#include <jack/jack.h>
#include <errno.h>
#include "jackruby.h"

void fake_free(void *mem) {
    return;
}

void jclient_free(void *mem) {
    jclient_t *cself = (jclient_t *) mem;
    if(!cself->closed) {
	j_client_close(cself->client);
    }
    xfree(cself);
}

#ifdef RUBY_THREADS

static VALUE j_mutex_lock_blocking_f(void *arg) {
    pthread_mutex_t *mutex = arg;
    return (VALUE) pthread_mutex_lock(mutex);
}

int j_mutex_lock(pthread_mutex_t *mutex) {
    return (int) rb_thread_blocking_region(j_mutex_lock_blocking_f, mutex, JACK_RUBY_UBF, 0);
}

// #else
// defined in header

#endif // #ifdef RUBY_THREADS

int j_mutex_init(pthread_mutex_t *mutex) {
    pthread_mutexattr_t attr;
    int r;
    r = pthread_mutexattr_init(&attr);
    if(r != 0) {
	return r;
    }
    r = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    if(r != 0) {
	return r;
    }
    r = pthread_mutex_init(mutex, &attr);
    if(r != 0) {
	return r;
    }
    pthread_mutexattr_destroy(&attr);
    return 0;
}

inline int is_true(VALUE t) {
    return ((NIL_P(t) || t == Qfalse) ? 0 : 1);
}

static VALUE jclient_alloc(VALUE klass) {
    return Data_Wrap_Struct(klass, 0, free, 0);
}

static VALUE jport_alloc(VALUE klass) {
    return Data_Wrap_Struct(klass, 0, free, 0);
}

#ifdef RUBY_THREADS

struct jack_client_port {
    jack_client_t *client;
    jack_port_t *port;
};

#endif

#ifdef RUBY_THREADS

struct jclient_open_struct {
    const char *client_name;
    jack_options_t options;
    jack_status_t *status;
    char *server_name;
};

static VALUE jclient_open_blocking_f(void *arg) {
    struct jclient_open_struct *open = (struct jclient_open_struct *) arg;
    jack_client_t *client;
    if(open->server_name == NULL) {
	client = jack_client_open(open->client_name, open->options, open->status);
    } else {
	client = jack_client_open(open->client_name, open->options, open->status, open->server_name);
    }
    return (VALUE) client;
}

static inline jack_client_t *jclient_open(const char *client_name, jack_options_t options, jack_status_t *status, char *server_name) {
    struct jclient_open_struct open;
    open.client_name = client_name;
    open.options = options;
    open.status = status;
    open.server_name = server_name;
    return (jack_client_t *) rb_thread_blocking_region(jclient_open_blocking_f, &open, JACK_RUBY_UBF, 0);
}

#else

static inline jack_client_t *jclient_open(const char *client_name, jack_options_t options, jack_status_t *status, char *server_name) {
    if(server_name == NULL) {
	return jack_client_open(client_name, options, status);
    } else {
	return jack_client_open(client_name, options, status, server_name);
    }
}

#endif

static VALUE jclient_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname, rflags, rservername;
    rb_scan_args(argc, argv, "21", &rname, &rflags, &rservername);
    char *name = StringValueCStr(rname);
    jack_options_t flags = NUM2JACKOPTIONST(rflags);
    char *servername = NIL_P(rservername) ? NULL : StringValueCStr(rservername);
    jclient_t *cself = ALLOC(jclient_t);
    jclient_init(name, flags, servername, cself);
    return Data_Wrap_Struct(klass, 0, jclient_free, cself);
}

void jclient_init(char *name, jack_options_t flags, char *server_name, jclient_t *cself) {
    jack_status_t status;
    cself->client = jclient_open(name, (server_name == NULL ? 0 : JackServerName) | flags, &status, server_name);
    if(cself->client == NULL) {
	xfree(cself);
	if(status & JackServerFailed) {
	    rb_raise(eJackServerFailed, "Unable to connect to the JACK server");
	} else if(status & JackServerError) {
	    rb_raise(eJackServerError, "Communication error with the JACK server");
	} else if(status & JackNoSuchClient) {
	    rb_raise(eJackNoSuchClient, "Requested client does not exist");
	} else if(status & JackLoadFailure) {
	    rb_raise(eJackLoadFailure, "Unable to load internal client");
	} else if(status & JackInitFailure) {
	    rb_raise(eJackInitFailure, "Unable to initialize client");
	} else if(status & JackShmFailure) {
	    rb_raise(eJackShmFailure, "Unable to access shared memory");
	} else if(status & JackVersionError) {
	    rb_raise(eJackVersionError, "Client's protocol version does not match");
	} else if(status & JackBackendError) {
	    rb_raise(eJackBackendError, "Backend error");
	} else if(status & JackClientZombie) {
	    rb_raise(eJackClientZombie, "Client zombie");
	} else if(status & JackInvalidOption) {
	    rb_raise(eJackInvalidOption, "The operation contained an invalid or unsupported option");
	} else if(status & JackNameNotUnique) {
	    rb_raise(eJackNameNotUnique, "The desired client name was not unique");
	} else {
	    rb_raise(eJackFailure, "Overall operation failed: %d", status);
	}
    }
    cself->closed = 0;
}

#define define_simple_jclient_f(rname, cname)				\
    static VALUE rname(VALUE self) {					\
	jclient_t *cself;						\
	Data_Get_Struct(self, jclient_t, cself);			\
	int r = cname(cself->client);					\
	if(r != 0) {							\
	    rb_raise(eJackFailure, "Overall operation failed: %d", r);	\
	}								\
	return self;							\
    }

#ifdef RUBY_THREADS

#define define_simple_jclient_blocking_f(rname, cname)			\
    static VALUE rname ## _blocking_f(void *arg) {			\
	jack_client_t *client = (jack_client_t *) arg;			\
	return (VALUE) cname(client);					\
    }									\
									\
    static VALUE rname(VALUE self) {					\
	jclient_t *cself;						\
	Data_Get_Struct(self, jclient_t, cself);			\
	int r = (int) rb_thread_blocking_region(rname ## _blocking_f, cself->client, JACK_RUBY_UBF, 0); \
	if(r != 0) {							\
	    rb_raise(eJackFailure, "Overall operation failed: %d", r);	\
	}								\
	return self;							\
    }

#else

#define define_simple_jclient_blocking_f(rname, cname) define_simple_jclient_f(rname, cname)

#endif // #ifdef RUBY_THREADS

#define define_set_jclient_f(rname, cname, vname, value)		\
    static VALUE rname(VALUE self, VALUE vname) {			\
	jclient_t *cself;						\
	Data_Get_Struct(self, jclient_t, cself);			\
	int r = cname(cself->client, value);				\
	if(r != 0) {							\
	    rb_raise(eJackFailure, "Overall operation failed: %d", r);	\
	}								\
	return vname;							\
    }

#ifdef RUBY_THREADS

#define define_set_jclient_blocking_f(rname, cname, vname, value, vtype) \
    struct rname ## _blocking_f_struct {				\
	jack_client_t *client;						\
	vtype arg;							\
    };									\
    									\
    static VALUE rname ## _blocking_f(void *arg) {			\
	struct rname ## _blocking_f_struct *sarg = (struct rname ## _blocking_f_struct *) arg; \
	return (VALUE) cname(sarg->client, sarg->arg);			\
    }									\
    									\
    static VALUE rname(VALUE self, VALUE vname) {			\
	struct rname ## _blocking_f_struct sarg;			\
	jclient_t *cself;						\
	Data_Get_Struct(self, jclient_t, cself);			\
	sarg.client = cself->client;					\
	sarg.arg = value;						\
	int r = (int) rb_thread_blocking_region(rname ## _blocking_f, &sarg, JACK_RUBY_UBF, 0); \
	if(r != 0) {							\
	    rb_raise(eJackFailure, "Overall operation failed: %d", r);	\
	}								\
	return self;							\
    }

#else

#define define_set_jclient_blocking_f(rname, cname, vname, value, vtype) define_set_jclient_f(rname, cname, vname, value)

#endif // #ifdef RUBY_THREADS

#define define_get_jclient_f(rname, value)				\
    static VALUE rname(VALUE self) {					\
	jclient_t *cself;						\
	Data_Get_Struct(self, jclient_t, cself);			\
	return value;							\
    }

#ifdef RUBY_THREADS

static VALUE jclient_close_blocking_f(void *arg) {
    jack_client_t *client = (jack_client_t *) arg;
    return (VALUE) jack_client_close(client);
}

static VALUE jclient_close(VALUE self) {
    jclient_t *cself;
    Data_Get_Struct(self, jclient_t, cself);
    int r = (int) rb_thread_blocking_region(jclient_close_blocking_f, cself->client, JACK_RUBY_UBF, 0);
    if(r != 0) {
	rb_raise(eJackFailure, "Overall operation failed: %d", r);
    }
    cself->closed = 1;
    return self;
}

#else

static VALUE jclient_close(VALUE self) {
    jclient_t *cself;
    Data_Get_Struct(self, jclient_t, cself);
    int r = jack_client_close(cself->client);
    if(r != 0) {
	rb_raise(eJackFailure, "Overall operation failed: %d", r);
    }
    cself->closed = 1;
    return self;
}

#endif

#ifdef RUBY_THREAD

// FIXME

// #else
// defined in header
#endif

define_get_jclient_f(jclient_get_name, rb_str_new_cstr(jack_get_client_name(cself->client)));
define_simple_jclient_blocking_f(jclient_activate, jack_activate);

#ifdef RUBY_THREADS

// FIXME

// #else
// defined in header

#endif

define_simple_jclient_blocking_f(jclient_deactivate, jack_deactivate);
define_get_jclient_f(jclient_is_realtime, jack_is_realtime(cself->client) == 0 ? Qfalse : Qtrue);

/* static VALUE jclient_cycle_wait_blocking_f(void *arg) { */
/*     jack_client_t *client = (jack_client_t *)arg; */
/*     return (VALUE) jack_cycle_wait(client); */
/* } */

/* static VALUE jclient_cycle_wait(VALUE self) { */
/*     jack_client_t *client; */
/*     Data_Get_Struct(self, jack_client_t, client); */
/*     jack_nframes_t frames = (jack_nframes_t) rb_thread_blocking_region(jclient_cycle_wait_blocking_f, client, JACK_RUBY_UBF, 0); */
/*     return JACKNFRAMEST2NUM(frames); */
/* } */

/* struct jclient_cycle_signal_struct { */
/*     jack_client_t *client; */
/*     int status; */
/* }; */

/* static VALUE jclient_cycle_signal_blocking_f(void *arg) { */
/*     struct jclient_cycle_signal_struct *args = (struct jclient_cycle_signal_struct *) arg; */
/*     jack_cycle_signal(args->client, args->status); */
/*     return Qnil; */
/* } */

/* static VALUE jclient_cycle_signal(int argc, VALUE *argv, VALUE self) { */
/*     struct jclient_cycle_signal_struct args; */
/*     VALUE status; */
/*     Data_Get_Struct(self, jack_client_t, args.client); */
/*     if(rb_scan_args(argc, argv, "1", &status) == 1) { */
/* 	args.status = NUM2INT(status); */
/*     } else { */
/* 	args.status = 0; */
/*     } */
/*     rb_thread_blocking_region(jclient_cycle_signal_blocking_f, &args, JACK_RUBY_UBF, 0); */
/*     return self; */
/* } */

define_set_jclient_blocking_f(jclient_set_freewheel_on, jack_set_freewheel, onoff, IS_TRUE(onoff), int);
define_set_jclient_blocking_f(jclient_set_buffer_size, jack_set_buffer_size, nframes, NUM2JACKNFRAMEST(nframes), jack_nframes_t);
define_get_jclient_f(jclient_get_sample_rate, JACKNFRAMEST2NUM(jack_get_sample_rate(cself->client)));
define_get_jclient_f(jclient_get_buffer_size, JACKNFRAMEST2NUM(jack_get_buffer_size(cself->client)));
define_get_jclient_f(jclient_get_cpu_load, rb_float_new(jack_cpu_load(cself->client)));

#ifdef RUBY_THREADS

struct j_client_port_register_struct {
    jack_client_t *client;
    char *port_name;
    char *port_type;
    unsigned long flags;
    unsigned long buffer_size;
};

static VALUE j_client_port_register_blocking_f(void *arg) {
    struct jclient_port_register_struct *sarg = (struct jclient_port_register_struct *) arg;
    return (VALUE) jack_port_register(sarg->client, sarg->port_name, sarg->port_type, sarg->flags, sarg->buffer_size);
}

jack_port_t *j_client_port_register(jack_client_t *client, char *port_name, char *port_type, unsigned long flags, unsigned long buffer_size) {
    struct jclient_port_register_struct sarg;
    sarg.client = client;
    sarg.port_name = port_name;
    sarg.port_type = port_type;
    sarg.flags = flags;
    sarg.buffer_size = buffer_size;
    return (jack_port_t *) rb_thread_blocking_region(jclient_port_register_blocking_f, &sarg, JACK_RUBY_UBF, 0);
}

// #else
// defined in header

#endif // #ifdef RUBY_THREADS

/* static VALUE jclient_port_register(VALUE self, VALUE port_name, VALUE flags, VALUE buffer_size) { */
/*     jack_client_t *client; */
/*     Data_Get_Struct(self, jack_client_t, client); */
/*     jack_port_t *port; */
/*     port = jack_port_register(client, StringValueCStr(port_name), JACK_DEFAULT_AUDIO_TYPE, NUM2ULONG(flags), NUM2ULONG(buffer_size)); */
/*     if(port == NULL) { */
/* 	rb_raise(eJackFailure, "Overall operation failed"); */
/*     } */
/*     return Data_Wrap_Struct(cJackPort, 0, fake_free, port); */
/* } */

#ifdef RUBY_THREADS

static VALUE j_client_port_unregister_blocking_f(void *arg) {
    struct jack_client_port *sarg = (struct jack_client_port *) arg;
    return (VALUE) jack_port_unregister(sarg->client, sarg->port);
}

int j_client_port_unregister(jack_client_t *client, jack_port_t *port) {
    struct jack_client_port sarg;
    sarg.client = client;
    sarg.port = port;
    return (int) rb_thread_blocking_region(jclient_port_unregister_blocking_f, &sarg, JACK_RUBY_UBF, 0);
}

// #else
// defined in header

#endif // #ifdef RUBY_THREADS

/* static VALUE jclient_port_unregister(VALUE self, VALUE r_port) { */
/*     jack_client_t *client; */
/*     jack_port_t *port; */
/*     Data_Get_Struct(self, jack_client_t, client); */
/*     Data_Get_Struct(r_port, jack_port_t, port); */
/*     int r = jack_port_unregister(client, port); */
/*     if(r != 0) { */
/* 	rb_raise(eJackFailure, "Overall operation failed: %d", r); */
/*     } */
/*     return self; */
/* } */

/* static VALUE jport_get_buffer(VALUE self, VALUE r_nframes) { */
/*     jack_default_audio_sample_t *buffer; */
/*     jack_nframes_t nframes = NUM2JACKNFRAMEST(r_nframes); */
/*     jack_port_t *port; */
/*     Data_Get_Struct(self, jack_port_t, port); */
/*     buffer = (jack_default_audio_sample_t *) jack_port_get_buffer(port, nframes); */
/*     if(buffer == NULL) { */
/* 	rb_raise(eJackFailure, "Overall operation failed"); */
/*     } */
/*     struct NARRAY *ary = ALLOC(struct NARRAY); */
/*     ary->shape = ALLOCA_N(int,1); */
/*     ary->ptr = (void *) buffer; */
/*     ary->rank = 1; */
/*     ary->total = nframes; */
/*     ary->type = NA_SFLOAT; */
/*     ary->ref = Qfalse; */
/*     return Data_Wrap_Struct(cNArray, 0, r_jack_narray_free, ary); */
/* } */

#define define_set_jport_f(rname, cname, vname, value)		\
    static VALUE rname(VALUE self, VALUE vname) {			\
	jack_port_t *port;						\
	Data_Get_Struct(self, jack_port_t, port);			\
	int r = cname(port, value);					\
	if(r != 0) {							\
	    rb_raise(eJackFailure, "Overall operation failed: %d", r);	\
	}								\
	return vname;							\
    }

#define define_void_set_jport_f(rname, cname, vname, value)		\
    static VALUE rname(VALUE self, VALUE vname) {			\
	jack_port_t *port;						\
	Data_Get_Struct(self, jack_port_t, port);			\
	cname(port, value);						\
	return vname;							\
    }

#ifdef RUBY_THREADS

#define define_set_jport_blocking_f(rname, cname, vname, value, vtype)	\
    struct rname ## _blocking_f_struct {				\
	jack_port_t *port;						\
	vtype arg;							\
    };									\
									\
    static VALUE rname ## _blocking_f(void *arg) {			\
	struct rname ## _blocking_f_struct *sarg = (struct rname ## _blocking_f_struct *) arg; \
	return (VALUE) cname(sarg->port, sarg->arg);			\
    }									\
									\
    static VALUE rname(VALUE self, VALUE vname) {			\
	struct rname ## _blocking_f_struct sarg;			\
	Data_Get_Struct(self, jack_port_t, sarg.port);		\
	sarg.arg = value;						\
	int r = (int) rb_thread_blocking_region(rname ## _blocking_f, &sarg, JACK_RUBY_UBF, 0); \
	if(r != 0) {							\
	    rb_raise(eJackFailure, "Overall operation failed: %d", r);	\
	}								\
	return self;							\
    }

#else

#define define_set_jport_blocking_f(rname, cname, vname, value, vtype) define_set_jport_f(rname, cname, vname, value)

#endif // #ifdef RUBY_THREADS

#define define_get_jport_f(rname, value)				\
    static VALUE rname(VALUE self) {					\
	jack_port_t *port;						\
	Data_Get_Struct(self, jack_port_t, port);			\
	return value;							\
    }

#ifdef RUBY_THREADS

#define define_get_jport_blocking_f(rname, cname, vname, value, vtype)	\
    static VALUE rname ## _blocking_f(void *arg) {			\
	jack_port_t *port = (jack_port_t *) arg;			\
	return (VALUE) cname(port);					\
    }									\
									\
    static VALUE rname(VALUE self) {					\
	jack_port_t *port;						\
	Data_Get_Struct(self, jack_port_t, port);			\
	vtype vname = (vtype) rb_thread_blocking_region(rname ## _blocking_f, port, JACK_RUBY_UBF, 0); \
	return value;							\
    }

#else

#define define_get_jport_blocking_f(rname, cname, vname, value, vtype)	\
    static VALUE rname(VALUE self) {					\
	jack_port_t *port;						\
	Data_Get_Struct(self, jack_port_t, port);			\
	vtype vname = cname(port);					\
	return value;							\
    }

#endif // #ifdef RUBY_THREADS


define_get_jport_f(jport_get_name, rb_str_new_cstr(jack_port_name(port)))
define_get_jport_f(jport_get_short_name, rb_str_new_cstr(jack_port_short_name(port)))
define_get_jport_f(jport_get_flags, INT2NUM(jack_port_flags(port)))
define_get_jport_f(jport_get_type, rb_str_new_cstr(jack_port_type(port)))

static VALUE jclient_is_mine(VALUE self, VALUE rport) {
    jclient_t *cself;
    jack_port_t *port;
    Data_Get_Struct(self, jclient_t, cself);
    if(!KIND_OF(rport, cJackPort)) {
	rb_raise(rb_eArgError, "must provide port");
    }
    Data_Get_Struct(rport, jack_port_t, port);
    return jack_port_is_mine(cself->client, port) ? Qtrue : Qfalse;
}

define_get_jport_blocking_f(jport_is_connected, jack_port_connected, connected, connected ? Qtrue : Qfalse, int)

#ifdef RUBY_THREADS

struct jport_is_connected_to_struct {
    jack_port_t *port;
    char *other;
};

static VALUE jport_is_connected_to_blocking_f(void *arg) {
    struct jport_is_connected_to_struct *sarg = (struct jport_is_connected_to_struct *) arg;
    return (VALUE) jack_port_connected_to(sarg->port, sarg->other);
}

static VALUE jport_is_connected_to(VALUE self, VALUE port_name) {
    struct jport_is_connected_to_struct sarg;
    Data_Get_Struct(self, jack_port_t, sarg.port);
    sarg.other = StringValueCStr(port_name);
    return ((int) rb_thread_blocking_region(jport_is_connected_to_blocking_f, &sarg, JACK_RUBY_UBF, 0)) ? Qtrue : Qfalse;
}

#else

static VALUE jport_is_connected_to(VALUE self, VALUE port_name) {
    jack_port_t *port;
    Data_Get_Struct(self, jack_port_t, port);
    return jack_port_connected_to(port, StringValueCStr(port_name)) ? Qtrue : Qfalse;
}

#endif // #ifdef RUBY_THREADS

#ifdef RUBY_THREADS

static VALUE jport_get_connections_blocking_f(void *arg) {
    jack_port_t *port = (jack_port_t *) arg;
    return (VALUE) jack_port_get_connections(port);
}

static VALUE jport_get_connections(VALUE self) {
    jack_port_t *port;
    Data_Get_Struct(self, jack_port_t, port);
    const char **ports = (const char **) rb_thread_blocking_region(jport_get_connections_blocking_f, port, JACK_RUBY_UBF, 0);
    long i;
    if(ports == NULL) {
	return rb_ary_new2(0);
    }
    for(i = 0; ports[i] != NULL; i++);
    VALUE ret_array = rb_ary_new2(i);
    for(i = 0; ports[i] != NULL; i++) {
	rb_ary_push(ret_array, rb_str_new_cstr(ports[i]));
    }
    jack_free(ports);
    return ret_array;
}

#else

static VALUE jport_get_connections(VALUE self) {
    jack_port_t *port;
    Data_Get_Struct(self, jack_port_t, port);
    const char **ports = jack_port_get_connections(port);
    long i;
    if(ports == NULL) {
	return rb_ary_new2(0);
    }
    for(i = 0; ports[i] != NULL; i++); // count the ports
    VALUE ret_array = rb_ary_new2(i);
    for(i = 0; ports[i] != NULL; i++) {
	rb_ary_push(ret_array, rb_str_new_cstr(ports[i]));
    }
    jack_free(ports);
    return ret_array;
}

#endif // #ifdef RUBY_THREADS

#ifdef RUBY_THREADS

static VALUE jclient_get_all_connections_blocking_f(void *arg) {
    struct jack_client_port *sarg = (struct jack_client_port *) arg;
    return (VALUE) jack_port_get_all_connections(sarg->client, sarg->port);
}

static VALUE jclient_get_all_connections(VALUE self, VALUE rport) {
    struct jack_client_port sarg;
    jclient_t *cself;
    Data_Get_Struct(self, jclient_t, cself);
    sarg.client = cself->client;
    if(!KIND_OF(rport, cJackPort)) {
	rb_raise(rb_eArgError, "must provide port");
    }
    Data_Get_Struct(rport, jack_port_t, sarg.port);
    const char **ports = (const char **) rb_thread_blocking_region(jclient_get_all_connections_blocking_f, &sarg, JACK_RUBY_UBF, 0);
    long i;
    for(i = 0; ports[i] != NULL; i++);
    VALUE ret_array = rb_ary_new2(i);
    for(i = 0; ports[i] != NULL; i++) {
	rb_ary_push(ret_array, rb_str_new_cstr(ports[i]));
    }
    jack_free(ports);
    return ret_array;
}

#else

static VALUE jclient_get_all_connections(VALUE self, VALUE rport) {
    jclient_t *cself;
    jack_port_t *port;
    Data_Get_Struct(self, jclient_t, cself);
    if(!KIND_OF(rport, cJackPort)) {
	rb_raise(rb_eArgError, "must provide port");
    }
    Data_Get_Struct(rport, jack_port_t, port);
    const char **ports = jack_port_get_all_connections(cself->client, port);
    long i;
    for(i = 0; ports[i] != NULL; i++);
    VALUE ret_array = rb_ary_new2(i);
    for(i = 0; ports[i] != NULL; i++) {
	rb_ary_push(ret_array, rb_str_new_cstr(ports[i]));
    }
    jack_free(ports);
    return ret_array;
}

#endif

define_get_jport_blocking_f(jport_get_latency, jack_port_get_latency, latency, JACKNFRAMEST2NUM(latency), jack_nframes_t)

#ifdef RUBY_THREADS

static VALUE jclient_get_total_latency_blocking_f(void *arg) {
    struct jack_client_port *sarg = (struct jack_client_port *) arg;
    return (VALUE) jack_port_get_total_latency(sarg->client, sarg->port);
}

static VALUE jclient_get_total_latency(VALUE self, VALUE rport) {
    jclient_t *cself;
    struct jack_client_port sarg;
    Data_Get_Struct(self, jclient_t, cself);
    sarg.client = cself->client;
    if(!KIND_OF(rport, cJackPort)) {
	rb_raise(rb_eArgError, "must provide port");
    }
    Data_Get_Struct(rport, jack_port_t, sarg.port);
    return JACKNFRAMEST2NUM((jack_nframes_t) rb_thread_blocking_region(jclient_get_total_latency_blocking_f, &sarg, JACK_RUBY_UBF, 0));
}

#else

static VALUE jclient_get_total_latency(VALUE self, VALUE rport) {
    jclient_t *cself;
    jack_port_t *port;
    Data_Get_Struct(self, jclient_t, cself);
    if(!KIND_OF(rport, cJackPort)) {
	rb_raise(rb_eArgError, "must provide port");
    }
    Data_Get_Struct(rport, jack_port_t, port);
    return JACKNFRAMEST2NUM(jack_port_get_total_latency(cself->client, port));
}

#endif // #ifdef RUBY_THREADS

define_void_set_jport_f(jport_set_latency, jack_port_set_latency, latency, NUM2JACKNFRAMEST(latency))

#ifdef RUBY_THREADS

static VALUE jclient_recompute_total_latency_blocking_f(void *arg) {
    struct jack_client_port *sarg = (struct jack_client_port *) arg;
    return (VALUE) jack_recompute_total_latency(sarg->client, sarg->port);
}

static VALUE jclient_recompute_total_latency(VALUE self, VALUE rport) {
    jclient_t *cself;
    struct jack_client_port sarg;
    Data_Get_Struct(self, jack_client_t, cself);
    sarg.client = cself->client;
    if(!KIND_OF(rport, cJackPort)) {
	rb_raise(rb_eArgError, "must provide port");
    }
    Data_Get_Struct(rport, jack_port_t, sarg.port);
    int r = (int) rb_thread_blocking_region(jclient_recompute_total_latency_blocking_f, &sarg, JACK_RUBY_UBF, 0);
    if(r != 0) {
	rb_raise(eJackFailure, "Overall operation failed: %d", r);
    }
    return self;
}

#else

static VALUE jclient_recompute_total_latency(VALUE self, VALUE rport) {
    jclient_t *cself;
    jack_port_t *port;
    Data_Get_Struct(self, jclient_t, cself);
    if(!KIND_OF(rport, cJackPort)) {
	rb_raise(rb_eArgError, "must provide port");
    }
    Data_Get_Struct(rport, jack_port_t, port);
    int r = jack_recompute_total_latency(cself->client, port);
    if(r != 0) {
	rb_raise(eJackFailure, "Overall operation failed: %d", r);
    }
    return self;
}

#endif // #ifdef RUBY_THREADS

define_simple_jclient_blocking_f(jclient_recompute_total_latencies, jack_recompute_total_latencies)
define_set_jport_blocking_f(jport_set_name, jack_port_set_name, name, StringValueCStr(name), char *)
define_set_jport_f(jport_alias, jack_port_set_alias, alias, StringValueCStr(alias))
define_set_jport_f(jport_unalias, jack_port_unset_alias, alias, StringValueCStr(alias))

static VALUE jport_get_aliases(VALUE self) {
    jack_port_t *port;
    Data_Get_Struct(self, jack_port_t, port);
    char *aliases[2];
    int s = jack_client_name_size() + jack_port_name_size();
    int i;
    for(i = 0; i < 2; i++) {
	aliases[i] = alloca(s);
	aliases[i][0] = '\0';
    }
    int r = jack_port_get_aliases(port, aliases);
    VALUE ret_array = rb_ary_new2(r);
    for(i = 0; i < 2; i++) {
	if(aliases[i][0] != '\0') {
	    rb_ary_push(ret_array, rb_str_new_cstr(aliases[i]));
	}
    }
    return ret_array;
}

define_set_jport_blocking_f(jport_request_monitor, jack_port_request_monitor, onoff, IS_TRUE(onoff), int)

#ifdef RUBY_THREADS

struct jclient_request_monitor_by_name_struct {
    jack_client_t *client;
    char *portname;
    int onoff;
};

static VALUE jclient_request_monitor_by_name_blocking_f(void *arg) {
    struct jclient_request_monitor_by_name_struct *sarg = (struct jclient_request_monitor_by_name_struct *) arg;
    return (VALUE) jack_port_request_monitor_by_name(sarg->client, sarg->portname, sarg->onoff);
}

static VALUE jclient_request_monitor_by_name(VALUE self, VALUE portname, VALUE onoff) {
    jclient_t *cself;
    struct jclient_request_monitor_by_name_struct sarg;
    Data_Get_Struct(self, jclient_t, cself);
    sarg.client = cself->client;
    sarg.portname = StringValueCStr(portname);
    sarg.onoff = IS_TRUE(onoff);
    int r = (int) rb_thread_blocking_region(jclient_request_monitor_by_name_blocking_f, &sarg, JACK_RUBY_UBF, 0);
    if(r != 0) {
	rb_raise(eJackFailure, "Overall operation failed: %d", r);
    }
    return self;
}

#else

static VALUE jclient_request_monitor_by_name(VALUE self, VALUE portname, VALUE onoff) {
    jclient_t *cself;
    Data_Get_Struct(self, jclient_t, cself);
    int r = jack_port_request_monitor_by_name(cself->client, StringValueCStr(portname), IS_TRUE(onoff));
    if(r != 0) {
	rb_raise(eJackFailure, "Overall operation failed: %d", r);
    }
    return self;
}

#endif // #ifdef RUBY_THREADS

define_set_jport_f(jport_ensure_monitor, jack_port_ensure_monitor, onoff, IS_TRUE(onoff))

define_get_jport_f(jport_is_monitoring_input, jack_port_monitoring_input(port) ? Qtrue : Qfalse)

#ifdef RUBY_THREADS

struct jclient_src_dst {
    jack_client_t *client;
    char *source_port;
    char *destination_port;
};

static VALUE jclient_connect_blocking_f(void *arg) {
    struct jclient_src_dst *sarg = (struct jclient_src_dst *) arg;
    return (VALUE) jack_connect(sarg->client, sarg->source_port, sarg->destination_port);
}

VALUE jclient_connect(VALUE self, VALUE source_port, VALUE destination_port) {
    jclient_t *cself;
    struct jclient_src_dst sarg;
    Data_Get_Struct(self, jclient_t, cself);
    sarg.client = cself->client;
    if(KIND_OF(source_port,cJackPort)) {
	jack_port_t *port;
	Data_Get_Struct(source_port, jack_port_t, port);
	sarg.source_port = jack_port_name(port);
    } else {
	sarg.source_port = StringValueCStr(source_port);
    }
    if(KIND_OF(destination_port,cJackPort)) {
	jack_port_t *port;
	Data_Get_Struct(destination_port, jack_port_t, port);
	sarg.destination_port = jack_port_name(port);
    } else {
	sarg.destination_port = StringValueCStr(destination_port);
    }
    int r = (int) rb_thread_blocking_region(jclient_connect_blocking_f, &sarg, JACK_RUBY_UBF, 0);
    if(r != 0) {
	if(r == EEXIST) {
	    rb_raise(eJackFailure, "Overall operation failed: connection already made");
	} else {
	    rb_raise(eJackFailure, "Overall operation failed: %d", r);
	}
    }
    return self;
}

static VALUE jclient_port_disconnect_blocking_f(void *arg) {
    struct jack_client_port *sarg = (struct jack_client_port *) arg;
    return (VALUE) jack_port_disconnect(sarg->client, sarg->port);
}

static VALUE jclient_disconnect_blocking_f(void *arg) {
    struct jclient_src_dst *sarg = (struct jclient_src_dst *) arg;
    return (VALUE) jack_disconnect(sarg->client, sarg->source_port, sarg->destination_port);
}

static VALUE jclient_disconnect(int argc, VALUE *argv, VALUE self) {
    jclient_t *cself;
    Data_Get_Struct(self, jack_client_t, cself);
    int r;
    VALUE source_port, destination_port;
    r = rb_scan_args(argc, argv, "11", &source_port, &destination_port);
    if(r == 1) {
	if(!KIND_OF(source_port, cJackPort)) {
	    rb_raise(rb_eArgError, "must provide port");
	}
	struct jack_client_port sarg;
	sarg.client = cself->client;
	Data_Get_Struct(source_port, jack_port_t, sarg.port);
 	r = (int) rb_thread_blocking_region(jclient_port_disconnect_blocking_f, &sarg, JACK_RUBY_UBF, 0);
    } else {
	struct jclient_src_dst sarg;
	sarg.client = cself->client;
	sarg.source_port = StringValueCStr(source_port);
	sarg.destination_port = StringValueCStr(destination_port);
	r = (int) rb_thread_blocking_region(jclient_disconnect_blocking_f, &sarg, JACK_RUBY_UBF, 0);
    }
    if(r != 0) {
	rb_raise(eJackFailure, "Overall operation failed: %d", r);
    }
    return self;
}

#else

VALUE jclient_connect(VALUE self, VALUE rsource_port, VALUE rdestination_port) {
    jclient_t *cself;
    Data_Get_Struct(self, jclient_t, cself);
    char *source_port;
    char *destination_port;
    if(KIND_OF(rsource_port,cJackPort)) {
	jack_port_t *port;
	Data_Get_Struct(rsource_port, jack_port_t, port);
	source_port = jack_port_name(port);
    } else {
	source_port = StringValueCStr(rsource_port);
    }
    if(KIND_OF(rdestination_port,cJackPort)) {
	jack_port_t *port;
	Data_Get_Struct(rdestination_port, jack_port_t, port);
	destination_port = jack_port_name(port);
    } else {
	destination_port = StringValueCStr(rdestination_port);
    }
    int r = jack_connect(cself->client, source_port, destination_port);
    if(r != 0) {
	if(r == EEXIST) {
	    rb_raise(eJackFailure, "Overall operation failed: connection already made");
	} else {
	    rb_raise(eJackFailure, "Overall operation failed: %d", r);
	}
    }
    return self;
}

static VALUE jclient_disconnect(int argc, VALUE *argv, VALUE self) {
    jclient_t *cself;
    Data_Get_Struct(self, jclient_t, cself);
    int r;
    VALUE source_port, destination_port;
    r = rb_scan_args(argc, argv, "11", &source_port, &destination_port);
    if(r == 1) {
	if(!KIND_OF(source_port, cJackPort)) {
	    rb_raise(rb_eArgError, "must provide port");
	}
	jack_port_t *port;
	Data_Get_Struct(source_port, jack_port_t, port);
 	r = jack_port_disconnect(cself->client, port);
    } else {
	r = jack_disconnect(cself->client, StringValueCStr(source_port), StringValueCStr(destination_port));
    }
    if(r != 0) {
	rb_raise(eJackFailure, "Overall operation failed: %d", r);
    }
    return self;
}

#endif // #ifdef RUBY_THREADS

static VALUE jclient_find_ports(int argc, VALUE *argv, VALUE self) {
    jclient_t *cself;
    Data_Get_Struct(self, jclient_t, cself);
    VALUE port_name_pattern, type_name_pattern, flags;
    rb_scan_args(argc, argv, "03", &port_name_pattern, &type_name_pattern, &flags);
    const char **ports = jack_get_ports(cself->client, port_name_pattern == Qnil ? NULL : StringValueCStr(port_name_pattern),
					type_name_pattern == Qnil ? NULL : StringValueCStr(type_name_pattern),
					flags == Qnil ? 0 : NUM2ULONG(flags));
    if(ports == NULL) {
	return rb_ary_new2(0);
    }
    long i;
    for(i = 0; ports[i] != NULL; i++);
    VALUE ret_array = rb_ary_new2(i);
    for(i = 0; ports[i] != NULL; i++) {
	rb_ary_push(ret_array, rb_str_new_cstr(ports[i]));
    }
    jack_free(ports);
    return ret_array;
}

static VALUE jclient_get_port_by_name(VALUE self, VALUE port_name) {
    jclient_t *cself;
    Data_Get_Struct(self, jclient_t, cself);
    jack_port_t *port = jack_port_by_name(cself->client, StringValueCStr(port_name));
    if(port == NULL) {
	return Qnil;
    }
    return Data_Wrap_Struct(cJackPort, 0, fake_free, port);
}

static VALUE jclient_get_port_by_id(VALUE self, VALUE port_id) {
    jclient_t *cself;
    Data_Get_Struct(self, jclient_t, cself);
    jack_port_t *port = jack_port_by_id(cself->client, NUM2JACKPORTIDT(port_id));
    if(port == NULL) {
	return Qnil;
    }
    return Data_Wrap_Struct(cJackPort, 0, fake_free, port);
}

define_get_jclient_f(jclient_get_frames_since_cycle_start, JACKNFRAMEST2NUM(jack_frames_since_cycle_start(cself->client)))
define_get_jclient_f(jclient_get_frame_time, JACKNFRAMEST2NUM(jack_frame_time(cself->client)))
define_get_jclient_f(jclient_get_last_frame_time, JACKNFRAMEST2NUM(jack_last_frame_time(cself->client)))

#define JACK_TIME_SCALEDOWN 0.000001
#define JACK_TIME_SCALEUP 1000000.0

static VALUE jclient_frames_to_time(VALUE self, VALUE nframes) {
    jclient_t *cself;
    Data_Get_Struct(self, jclient_t, cself);
    return DBL2NUM((((double) jack_frames_to_time(cself->client, NUM2JACKNFRAMEST(nframes))) * JACK_TIME_SCALEDOWN));
}

static VALUE jclient_time_to_frames(VALUE self, VALUE time) {
    jack_client_t *client;
    Data_Get_Struct(self, jack_client_t, client);
    return JACKNFRAMEST2NUM(jack_time_to_frames(client, (jack_time_t) (NUM2DBL(time) * JACK_TIME_SCALEUP)));
}

static VALUE jclient_get_time(VALUE self) {
    return DBL2NUM(((double) jack_get_time()) * JACK_TIME_SCALEDOWN);
}

static VALUE jclient_max_name_size(VALUE self) {
    return INT2NUM(jack_client_name_size());
}

static VALUE jport_max_name_size(VALUE self) {
    return INT2NUM(jack_port_name_size());
}

void Init_jackruby() {
    mJack = rb_define_module("Jack");

    cJackClient = rb_define_class_under(mJack, "Client", rb_cObject);
    cJackPort = rb_define_class_under(mJack, "Port", rb_cObject);

    eJackFailure = rb_define_class_under(mJack, "Failure", rb_eRuntimeError);
    eJackServerFailed = rb_define_class_under(mJack, "ServerFailed", eJackFailure);
    eJackServerError = rb_define_class_under(mJack, "ServerError", eJackFailure);
    eJackNoSuchClient = rb_define_class_under(mJack, "NoSuchClient", eJackFailure);
    eJackLoadFailure = rb_define_class_under(mJack, "LoadFailure", eJackFailure);
    eJackInitFailure = rb_define_class_under(mJack, "InitFailure", eJackFailure);
    eJackShmFailure = rb_define_class_under(mJack, "ShmFailure", eJackFailure);
    eJackVersionError = rb_define_class_under(mJack, "VersionError", eJackFailure);
    eJackBackendError = rb_define_class_under(mJack, "BackendError", eJackFailure);
    eJackClientZombie = rb_define_class_under(mJack, "ClientZombie", eJackFailure);
    eJackInvalidOption = rb_define_class_under(mJack, "InvalidOption", eJackFailure);
    eJackNameNotUnique = rb_define_class_under(mJack, "NameNotUnique", eJackFailure);

    rb_define_alloc_func(cJackClient, jclient_alloc);
    rb_define_alloc_func(cJackPort, jport_alloc);

    rb_define_singleton_method(cJackClient, "MAX_NAME_SIZE", jclient_max_name_size, 0);
    rb_define_singleton_method(cJackPort, "MAX_NAME_SIZE", jport_max_name_size, 0);

    rb_define_singleton_method(cJackClient, "new", jclient_new, -1);
    rb_define_method(cJackClient, "close", jclient_close, 0);
    rb_define_method(cJackClient, "name", jclient_get_name, 0);
    rb_define_method(cJackClient, "activate", jclient_activate, 0);
    rb_define_method(cJackClient, "deactivate", jclient_deactivate, 0);
    rb_define_method(cJackClient, "realtime?", jclient_is_realtime, 0);
    /* rb_define_method(cJackClient, "cycle_wait", jclient_cycle_wait, 0); */
    /* rb_define_method(cJackClient, "cycle_signal", jclient_cycle_signal, -1); */
    rb_define_method(cJackClient, "freewheel_on=", jclient_set_freewheel_on, 1);
    rb_define_method(cJackClient, "buffer_size=", jclient_set_buffer_size, 1);
    rb_define_method(cJackClient, "sample_rate", jclient_get_sample_rate, 0);
    rb_define_method(cJackClient, "buffer_size", jclient_get_buffer_size, 0);
    rb_define_method(cJackClient, "cpu_load", jclient_get_cpu_load, 0);
    /* rb_define_method(cJackClient, "port_register", jclient_port_register, 3); */
    /* rb_define_method(cJackClient, "port_unregister", jclient_port_unregister, 1); */
    /* rb_define_method(cJackPort, "get_buffer", jport_get_buffer, 1); */
    rb_define_method(cJackPort, "name", jport_get_name, 0);
    rb_define_method(cJackPort, "short_name", jport_get_short_name, 0);
    rb_define_method(cJackPort, "flags", jport_get_flags, 0);
    rb_define_method(cJackPort, "type", jport_get_type, 0);
    rb_define_method(cJackClient, "mine?", jclient_is_mine, 1);
    rb_define_method(cJackPort, "connected?", jport_is_connected, 0);
    rb_define_method(cJackPort, "connected_to?", jport_is_connected_to, 1);
    rb_define_method(cJackPort, "connections", jport_get_connections, 0);
    rb_define_method(cJackClient, "all_connections", jclient_get_all_connections, 1);
    rb_define_method(cJackPort, "latency", jport_get_latency, 0);
    rb_define_method(cJackClient, "total_latency", jclient_get_total_latency, 1);
    rb_define_method(cJackPort, "latency=", jport_set_latency, 1);
    rb_define_method(cJackClient, "recompute_total_latency", jclient_recompute_total_latency, 1);
    rb_define_method(cJackClient, "recompute_total_latencies", jclient_recompute_total_latencies, 0);
    rb_define_method(cJackPort, "name=", jport_set_name, 1);
    rb_define_method(cJackPort, "alias", jport_alias, 1);
    rb_define_method(cJackPort, "unalias", jport_unalias, 1);
    rb_define_method(cJackPort, "aliases", jport_get_aliases, 0);
    rb_define_method(cJackPort, "request_monitor", jport_request_monitor, 1);
    rb_define_method(cJackClient, "request_monitor_by_name", jclient_request_monitor_by_name, 2);
    rb_define_method(cJackPort, "ensure_monitor", jport_ensure_monitor, 1);
    rb_define_method(cJackPort, "monitoring_input?", jport_is_monitoring_input, 0);
    rb_define_method(cJackClient, "connect", jclient_connect, 2);
    rb_define_method(cJackClient, "disconnect", jclient_disconnect, -1);
    rb_define_method(cJackClient, "find_ports", jclient_find_ports, -1);
    rb_define_method(cJackClient, "ports", jclient_find_ports, -1);
    rb_define_method(cJackClient, "get_port_by_name", jclient_get_port_by_name, 1);
    rb_define_method(cJackClient, "get_port_by_id", jclient_get_port_by_id, 1);
    rb_define_method(cJackClient, "frames_since_cycle_start", jclient_get_frames_since_cycle_start, 0);
    rb_define_method(cJackClient, "time", jclient_get_time, 0);
    rb_define_method(cJackClient, "frame_time", jclient_get_frame_time, 0);
    rb_define_method(cJackClient, "last_frame_time", jclient_get_last_frame_time, 0);
    rb_define_method(cJackClient, "frames_to_time", jclient_frames_to_time, 1);
    rb_define_method(cJackClient, "time_to_frames", jclient_time_to_frames, 1);
    rb_define_method(cJackClient, "time_to_frames", jclient_time_to_frames, 1);

    Init_synths();
    Init_constant();
    Init_envelope_generator();
    Init_falling_saw();
    Init_instrument();
    Init_key();
    Init_mixer();
    Init_modulator();
    Init_rising_saw();
    Init_scale();
    Init_sequencer();
    Init_sine();
    Init_square();
    Init_timer();
    Init_triangle();
}
