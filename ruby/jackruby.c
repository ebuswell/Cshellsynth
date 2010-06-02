#include <ruby.h>
#include <jack/jack.h>
#include <errno.h>
#include <cshellsynth/jclient.h>
#include "jackruby.h"

void fake_free(void *mem) {
    return;
}

static void jclient_free(void *mem) {
    jclient_t *cself = (jclient_t *) mem;
    jclient_destroy(cself);
    xfree(cself);
}

inline int is_true(VALUE t) {
    return ((NIL_P(t) || t == Qfalse) ? 0 : 1);
}

static VALUE jr_client_alloc(VALUE klass) {
    return Data_Wrap_Struct(klass, 0, fake_free, 0);
}

static VALUE jr_port_alloc(VALUE klass) {
    return Data_Wrap_Struct(klass, 0, fake_free, 0);
}

#ifdef RUBY_THREADS

struct jack_client_port {
    jack_client_t *client;
    jack_port_t *port;
};

#endif

static VALUE jr_client_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rname, rflags, rservername;
    rb_scan_args(argc, argv, "21", &rname, &rflags, &rservername);
    char *name = StringValueCStr(rname);
    jack_options_t flags = NUM2JACKOPTIONST(rflags);
    char *servername = NIL_P(rservername) ? NULL : StringValueCStr(rservername);
    jclient_t *cself = ALLOC(jclient_t);
    int r = jclient_init(cself, name, flags, servername);
    JR_CHECK_INIT_ERROR(cself, r);
    return Data_Wrap_Struct(klass, 0, jclient_free, cself);
}

#define define_simple_jr_client_f(rname, cname)				\
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

#define define_simple_jr_client_blocking_f(rname, cname)		\
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

#define define_simple_jr_client_blocking_f(rname, cname) define_simple_jr_client_f(rname, cname)

#endif // #ifdef RUBY_THREADS

#define define_set_jr_client_f(rname, cname, vname, value)		\
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

#define define_set_jr_client_blocking_f(rname, cname, vname, value, vtype) \
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

#define define_set_jr_client_blocking_f(rname, cname, vname, value, vtype) define_set_jr_client_f(rname, cname, vname, value)

#endif // #ifdef RUBY_THREADS

#define define_get_jr_client_f(rname, value)				\
    static VALUE rname(VALUE self) {					\
	jclient_t *cself;						\
	Data_Get_Struct(self, jclient_t, cself);			\
	return value;							\
    }

// define_simple_jr_client_blocking_f(jr_client_close, jack_client_close(client);

define_get_jr_client_f(jr_client_get_name, rb_str_new_cstr(jack_get_client_name(cself->client)));
define_simple_jr_client_blocking_f(jr_client_activate, jack_activate);
define_simple_jr_client_blocking_f(jr_client_deactivate, jack_deactivate);
define_get_jr_client_f(jr_client_is_realtime, jack_is_realtime(cself->client) == 0 ? Qfalse : Qtrue);
define_set_jr_client_blocking_f(jr_client_set_freewheel_on, jack_set_freewheel, onoff, IS_TRUE(onoff), int);
define_set_jr_client_blocking_f(jr_client_set_buffer_size, jack_set_buffer_size, nframes, NUM2JACKNFRAMEST(nframes), jack_nframes_t);
define_get_jr_client_f(jr_client_get_sample_rate, JACKNFRAMEST2NUM(jack_get_sample_rate(cself->client)));
define_get_jr_client_f(jr_client_get_buffer_size, JACKNFRAMEST2NUM(jack_get_buffer_size(cself->client)));
define_get_jr_client_f(jr_client_get_cpu_load, rb_float_new(jack_cpu_load(cself->client)));

#define define_set_jr_port_f(rname, cname, vname, value)		\
    static VALUE rname(VALUE self, VALUE vname) {			\
	jack_port_t *port;						\
	Data_Get_Struct(self, jack_port_t, port);			\
	int r = cname(port, value);					\
	if(r != 0) {							\
	    rb_raise(eJackFailure, "Overall operation failed: %d", r);	\
	}								\
	return vname;							\
    }

#define define_void_set_jr_port_f(rname, cname, vname, value)		\
    static VALUE rname(VALUE self, VALUE vname) {			\
	jack_port_t *port;						\
	Data_Get_Struct(self, jack_port_t, port);			\
	cname(port, value);						\
	return vname;							\
    }

#ifdef RUBY_THREADS

#define define_set_jr_port_blocking_f(rname, cname, vname, value, vtype) \
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
	Data_Get_Struct(self, jack_port_t, sarg.port);			\
	sarg.arg = value;						\
	int r = (int) rb_thread_blocking_region(rname ## _blocking_f, &sarg, JACK_RUBY_UBF, 0); \
	if(r != 0) {							\
	    rb_raise(eJackFailure, "Overall operation failed: %d", r);	\
	}								\
	return self;							\
    }

#else

#define define_set_jr_port_blocking_f(rname, cname, vname, value, vtype) define_set_jr_port_f(rname, cname, vname, value)

#endif // #ifdef RUBY_THREADS

#define define_get_jr_port_f(rname, value)				\
    static VALUE rname(VALUE self) {					\
	jack_port_t *port;						\
	Data_Get_Struct(self, jack_port_t, port);			\
	return value;							\
    }

#ifdef RUBY_THREADS

#define define_get_jr_port_blocking_f(rname, cname, vname, value, vtype) \
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

#define define_get_jr_port_blocking_f(rname, cname, vname, value, vtype) \
    static VALUE rname(VALUE self) {					\
	jack_port_t *port;						\
	Data_Get_Struct(self, jack_port_t, port);			\
	vtype vname = cname(port);					\
	return value;							\
    }

#endif // #ifdef RUBY_THREADS

define_get_jr_port_f(jr_port_get_name, rb_str_new_cstr(jack_port_name(port)))
define_get_jr_port_f(jr_port_get_short_name, rb_str_new_cstr(jack_port_short_name(port)))
define_get_jr_port_f(jr_port_get_flags, INT2NUM(jack_port_flags(port)))
define_get_jr_port_f(jr_port_get_type, rb_str_new_cstr(jack_port_type(port)))

static VALUE jr_client_is_mine(VALUE self, VALUE rport) {
    jclient_t *cself;
    jack_port_t *port;
    Data_Get_Struct(self, jclient_t, cself);
    if(!KIND_OF(rport, cJackPort)) {
	rb_raise(rb_eArgError, "must provide port");
    }
    Data_Get_Struct(rport, jack_port_t, port);
    return jack_port_is_mine(cself->client, port) ? Qtrue : Qfalse;
}

define_get_jr_port_blocking_f(jr_port_is_connected, jack_port_connected, connected, connected ? Qtrue : Qfalse, int)

#ifdef RUBY_THREADS

struct jr_port_is_connected_to_struct {
    jack_port_t *port;
    char *other;
};

static VALUE jr_port_is_connected_to_blocking_f(void *arg) {
    struct jr_port_is_connected_to_struct *sarg = (struct jr_port_is_connected_to_struct *) arg;
    return (VALUE) jack_port_connected_to(sarg->port, sarg->other);
}

static VALUE jr_port_is_connected_to(VALUE self, VALUE port_name) {
    struct jr_port_is_connected_to_struct sarg;
    Data_Get_Struct(self, jack_port_t, sarg.port);
    sarg.other = StringValueCStr(port_name);
    return ((int) rb_thread_blocking_region(jr_port_is_connected_to_blocking_f, &sarg, JACK_RUBY_UBF, 0)) ? Qtrue : Qfalse;
}

#else

static VALUE jr_port_is_connected_to(VALUE self, VALUE port_name) {
    jack_port_t *port;
    Data_Get_Struct(self, jack_port_t, port);
    return jack_port_connected_to(port, StringValueCStr(port_name)) ? Qtrue : Qfalse;
}

#endif // #ifdef RUBY_THREADS

#ifdef RUBY_THREADS

static VALUE jr_port_get_connections_blocking_f(void *arg) {
    jack_port_t *port = (jack_port_t *) arg;
    return (VALUE) jack_port_get_connections(port);
}

static VALUE jr_port_get_connections(VALUE self) {
    jack_port_t *port;
    Data_Get_Struct(self, jack_port_t, port);
    const char **ports = (const char **) rb_thread_blocking_region(jr_port_get_connections_blocking_f, port, JACK_RUBY_UBF, 0);
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

static VALUE jr_port_get_connections(VALUE self) {
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

static VALUE jr_client_get_all_connections_blocking_f(void *arg) {
    struct jack_client_port *sarg = (struct jack_client_port *) arg;
    return (VALUE) jack_port_get_all_connections(sarg->client, sarg->port);
}

static VALUE jr_client_get_all_connections(VALUE self, VALUE rport) {
    struct jack_client_port sarg;
    jclient_t *cself;
    Data_Get_Struct(self, jclient_t, cself);
    sarg.client = cself->client;
    if(!KIND_OF(rport, cJackPort)) {
	rb_raise(rb_eArgError, "must provide port");
    }
    Data_Get_Struct(rport, jack_port_t, sarg.port);
    const char **ports = (const char **) rb_thread_blocking_region(jr_client_get_all_connections_blocking_f, &sarg, JACK_RUBY_UBF, 0);
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

static VALUE jr_client_get_all_connections(VALUE self, VALUE rport) {
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

define_get_jr_port_blocking_f(jr_port_get_latency, jack_port_get_latency, latency, JACKNFRAMEST2NUM(latency), jack_nframes_t)

#ifdef RUBY_THREADS

static VALUE jr_client_get_total_latency_blocking_f(void *arg) {
    struct jack_client_port *sarg = (struct jack_client_port *) arg;
    return (VALUE) jack_port_get_total_latency(sarg->client, sarg->port);
}

static VALUE jr_client_get_total_latency(VALUE self, VALUE rport) {
    jclient_t *cself;
    struct jack_client_port sarg;
    Data_Get_Struct(self, jclient_t, cself);
    sarg.client = cself->client;
    if(!KIND_OF(rport, cJackPort)) {
	rb_raise(rb_eArgError, "must provide port");
    }
    Data_Get_Struct(rport, jack_port_t, sarg.port);
    return JACKNFRAMEST2NUM((jack_nframes_t) rb_thread_blocking_region(jr_client_get_total_latency_blocking_f, &sarg, JACK_RUBY_UBF, 0));
}

#else

static VALUE jr_client_get_total_latency(VALUE self, VALUE rport) {
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

define_void_set_jr_port_f(jr_port_set_latency, jack_port_set_latency, latency, NUM2JACKNFRAMEST(latency))

#ifdef RUBY_THREADS

static VALUE jr_client_recompute_total_latency_blocking_f(void *arg) {
    struct jack_client_port *sarg = (struct jack_client_port *) arg;
    return (VALUE) jack_recompute_total_latency(sarg->client, sarg->port);
}

static VALUE jr_client_recompute_total_latency(VALUE self, VALUE rport) {
    jclient_t *cself;
    struct jack_client_port sarg;
    Data_Get_Struct(self, jack_client_t, cself);
    sarg.client = cself->client;
    if(!KIND_OF(rport, cJackPort)) {
	rb_raise(rb_eArgError, "must provide port");
    }
    Data_Get_Struct(rport, jack_port_t, sarg.port);
    int r = (int) rb_thread_blocking_region(jr_client_recompute_total_latency_blocking_f, &sarg, JACK_RUBY_UBF, 0);
    if(r != 0) {
	rb_raise(eJackFailure, "Overall operation failed: %d", r);
    }
    return self;
}

#else

static VALUE jr_client_recompute_total_latency(VALUE self, VALUE rport) {
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

define_simple_jr_client_blocking_f(jr_client_recompute_total_latencies, jack_recompute_total_latencies)
define_set_jr_port_blocking_f(jr_port_set_name, jack_port_set_name, name, StringValueCStr(name), char *)
define_set_jr_port_f(jr_port_alias, jack_port_set_alias, alias, StringValueCStr(alias))
define_set_jr_port_f(jr_port_unalias, jack_port_unset_alias, alias, StringValueCStr(alias))

static VALUE jr_port_get_aliases(VALUE self) {
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

define_set_jr_port_blocking_f(jr_port_request_monitor, jack_port_request_monitor, onoff, IS_TRUE(onoff), int)

#ifdef RUBY_THREADS

struct jr_client_request_monitor_by_name_struct {
    jack_client_t *client;
    char *portname;
    int onoff;
};

static VALUE jr_client_request_monitor_by_name_blocking_f(void *arg) {
    struct jclient_request_monitor_by_name_struct *sarg = (struct jclient_request_monitor_by_name_struct *) arg;
    return (VALUE) jack_port_request_monitor_by_name(sarg->client, sarg->portname, sarg->onoff);
}

static VALUE jr_client_request_monitor_by_name(VALUE self, VALUE portname, VALUE onoff) {
    jclient_t *cself;
    struct jr_client_request_monitor_by_name_struct sarg;
    Data_Get_Struct(self, jclient_t, cself);
    sarg.client = cself->client;
    sarg.portname = StringValueCStr(portname);
    sarg.onoff = IS_TRUE(onoff);
    int r = (int) rb_thread_blocking_region(jr_client_request_monitor_by_name_blocking_f, &sarg, JACK_RUBY_UBF, 0);
    if(r != 0) {
	rb_raise(eJackFailure, "Overall operation failed: %d", r);
    }
    return self;
}

#else

static VALUE jr_client_request_monitor_by_name(VALUE self, VALUE portname, VALUE onoff) {
    jclient_t *cself;
    Data_Get_Struct(self, jclient_t, cself);
    int r = jack_port_request_monitor_by_name(cself->client, StringValueCStr(portname), IS_TRUE(onoff));
    if(r != 0) {
	rb_raise(eJackFailure, "Overall operation failed: %d", r);
    }
    return self;
}

#endif // #ifdef RUBY_THREADS

define_set_jr_port_f(jr_port_ensure_monitor, jack_port_ensure_monitor, onoff, IS_TRUE(onoff))

define_get_jr_port_f(jr_port_is_monitoring_input, jack_port_monitoring_input(port) ? Qtrue : Qfalse)

#ifdef RUBY_THREADS

struct jr_client_src_dst {
    jack_client_t *client;
    char *source_port;
    char *destination_port;
};

static VALUE jr_client_connect_blocking_f(void *arg) {
    struct jr_client_src_dst *sarg = (struct jr_client_src_dst *) arg;
    return (VALUE) jack_connect(sarg->client, sarg->source_port, sarg->destination_port);
}

VALUE jr_client_connect(VALUE self, VALUE source_port, VALUE destination_port) {
    jclient_t *cself;
    struct jr_client_src_dst sarg;
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
    int r = (int) rb_thread_blocking_region(jr_client_connect_blocking_f, &sarg, JACK_RUBY_UBF, 0);
    if(r != 0) {
	if(r == EEXIST) {
	    rb_raise(eJackFailure, "Overall operation failed: connection already made");
	} else {
	    rb_raise(eJackFailure, "Overall operation failed: %d", r);
	}
    }
    return self;
}

static VALUE jr_client_port_disconnect_blocking_f(void *arg) {
    struct jack_client_port *sarg = (struct jack_client_port *) arg;
    return (VALUE) jack_port_disconnect(sarg->client, sarg->port);
}

static VALUE jr_client_disconnect_blocking_f(void *arg) {
    struct jclient_src_dst *sarg = (struct jclient_src_dst *) arg;
    return (VALUE) jack_disconnect(sarg->client, sarg->source_port, sarg->destination_port);
}

static VALUE jr_client_disconnect(int argc, VALUE *argv, VALUE self) {
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
 	r = (int) rb_thread_blocking_region(jr_client_port_disconnect_blocking_f, &sarg, JACK_RUBY_UBF, 0);
    } else {
	struct jr_client_src_dst sarg;
	sarg.client = cself->client;
	sarg.source_port = StringValueCStr(source_port);
	sarg.destination_port = StringValueCStr(destination_port);
	r = (int) rb_thread_blocking_region(jr_client_disconnect_blocking_f, &sarg, JACK_RUBY_UBF, 0);
    }
    if(r != 0) {
	rb_raise(eJackFailure, "Overall operation failed: %d", r);
    }
    return self;
}

#else

VALUE jr_client_connect(VALUE self, VALUE rsource_port, VALUE rdestination_port) {
    jclient_t *cself;
    Data_Get_Struct(self, jclient_t, cself);
    char *source_port;
    char *destination_port;
    if(KIND_OF(rsource_port,cJackPort)) {
	jack_port_t *port;
	Data_Get_Struct(rsource_port, jack_port_t, port);
	source_port = (char *) jack_port_name(port);
    } else {
	source_port = StringValueCStr(rsource_port);
    }
    if(KIND_OF(rdestination_port,cJackPort)) {
	jack_port_t *port;
	Data_Get_Struct(rdestination_port, jack_port_t, port);
	destination_port = (char *) jack_port_name(port);
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

static VALUE jr_client_disconnect(int argc, VALUE *argv, VALUE self) {
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

static VALUE jr_client_find_ports(int argc, VALUE *argv, VALUE self) {
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

static VALUE jr_client_get_port_by_name(VALUE self, VALUE port_name) {
    jclient_t *cself;
    Data_Get_Struct(self, jclient_t, cself);
    jack_port_t *port = jack_port_by_name(cself->client, StringValueCStr(port_name));
    if(port == NULL) {
	return Qnil;
    }
    return Data_Wrap_Struct(cJackPort, 0, fake_free, port);
}

static VALUE jr_client_get_port_by_id(VALUE self, VALUE port_id) {
    jclient_t *cself;
    Data_Get_Struct(self, jclient_t, cself);
    jack_port_t *port = jack_port_by_id(cself->client, NUM2JACKPORTIDT(port_id));
    if(port == NULL) {
	return Qnil;
    }
    return Data_Wrap_Struct(cJackPort, 0, fake_free, port);
}

define_get_jr_client_f(jr_client_get_frames_since_cycle_start, JACKNFRAMEST2NUM(jack_frames_since_cycle_start(cself->client)))
define_get_jr_client_f(jr_client_get_frame_time, JACKNFRAMEST2NUM(jack_frame_time(cself->client)))
define_get_jr_client_f(jr_client_get_last_frame_time, JACKNFRAMEST2NUM(jack_last_frame_time(cself->client)))

#define JACK_TIME_SCALEDOWN 0.000001
#define JACK_TIME_SCALEUP 1000000.0

static VALUE jr_client_frames_to_time(VALUE self, VALUE nframes) {
    jclient_t *cself;
    Data_Get_Struct(self, jclient_t, cself);
    return DBL2NUM((((double) jack_frames_to_time(cself->client, NUM2JACKNFRAMEST(nframes))) * JACK_TIME_SCALEDOWN));
}

static VALUE jr_client_time_to_frames(VALUE self, VALUE time) {
    jack_client_t *client;
    Data_Get_Struct(self, jack_client_t, client);
    return JACKNFRAMEST2NUM(jack_time_to_frames(client, (jack_time_t) (NUM2DBL(time) * JACK_TIME_SCALEUP)));
}

static VALUE jr_client_get_time(VALUE self) {
    return DBL2NUM(((double) jack_get_time()) * JACK_TIME_SCALEDOWN);
}

static VALUE jr_client_max_name_size(VALUE self) {
    return INT2NUM(jack_client_name_size());
}

static VALUE jr_port_max_name_size(VALUE self) {
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

    rb_define_alloc_func(cJackClient, jr_client_alloc);
    rb_define_alloc_func(cJackPort, jr_port_alloc);

    rb_define_singleton_method(cJackClient, "MAX_NAME_SIZE", jr_client_max_name_size, 0);
    rb_define_singleton_method(cJackPort, "MAX_NAME_SIZE", jr_port_max_name_size, 0);

    rb_define_singleton_method(cJackClient, "new", jr_client_new, -1);
    // rb_define_method(cJackClient, "close", jr_client_close, 0);
    rb_define_method(cJackClient, "name", jr_client_get_name, 0);
    rb_define_method(cJackClient, "activate", jr_client_activate, 0);
    rb_define_method(cJackClient, "deactivate", jr_client_deactivate, 0);
    rb_define_method(cJackClient, "realtime?", jr_client_is_realtime, 0);
    rb_define_method(cJackClient, "freewheel_on=", jr_client_set_freewheel_on, 1);
    rb_define_method(cJackClient, "buffer_size=", jr_client_set_buffer_size, 1);
    rb_define_method(cJackClient, "sample_rate", jr_client_get_sample_rate, 0);
    rb_define_method(cJackClient, "buffer_size", jr_client_get_buffer_size, 0);
    rb_define_method(cJackClient, "cpu_load", jr_client_get_cpu_load, 0);
    rb_define_method(cJackPort, "name", jr_port_get_name, 0);
    rb_define_method(cJackPort, "short_name", jr_port_get_short_name, 0);
    rb_define_method(cJackPort, "flags", jr_port_get_flags, 0);
    rb_define_method(cJackPort, "type", jr_port_get_type, 0);
    rb_define_method(cJackClient, "mine?", jr_client_is_mine, 1);
    rb_define_method(cJackPort, "connected?", jr_port_is_connected, 0);
    rb_define_method(cJackPort, "connected_to?", jr_port_is_connected_to, 1);
    rb_define_method(cJackPort, "connections", jr_port_get_connections, 0);
    rb_define_method(cJackClient, "all_connections", jr_client_get_all_connections, 1);
    rb_define_method(cJackPort, "latency", jr_port_get_latency, 0);
    rb_define_method(cJackClient, "total_latency", jr_client_get_total_latency, 1);
    rb_define_method(cJackPort, "latency=", jr_port_set_latency, 1);
    rb_define_method(cJackClient, "recompute_total_latency", jr_client_recompute_total_latency, 1);
    rb_define_method(cJackClient, "recompute_total_latencies", jr_client_recompute_total_latencies, 0);
    rb_define_method(cJackPort, "name=", jr_port_set_name, 1);
    rb_define_method(cJackPort, "alias", jr_port_alias, 1);
    rb_define_method(cJackPort, "unalias", jr_port_unalias, 1);
    rb_define_method(cJackPort, "aliases", jr_port_get_aliases, 0);
    rb_define_method(cJackPort, "request_monitor", jr_port_request_monitor, 1);
    rb_define_method(cJackClient, "request_monitor_by_name", jr_client_request_monitor_by_name, 2);
    rb_define_method(cJackPort, "ensure_monitor", jr_port_ensure_monitor, 1);
    rb_define_method(cJackPort, "monitoring_input?", jr_port_is_monitoring_input, 0);
    rb_define_method(cJackClient, "connect", jr_client_connect, 2);
    rb_define_method(cJackClient, "disconnect", jr_client_disconnect, -1);
    rb_define_method(cJackClient, "find_ports", jr_client_find_ports, -1);
    rb_define_method(cJackClient, "ports", jr_client_find_ports, -1);
    rb_define_method(cJackClient, "get_port_by_name", jr_client_get_port_by_name, 1);
    rb_define_method(cJackClient, "get_port_by_id", jr_client_get_port_by_id, 1);
    rb_define_method(cJackClient, "frames_since_cycle_start", jr_client_get_frames_since_cycle_start, 0);
    rb_define_method(cJackClient, "time", jr_client_get_time, 0);
    rb_define_method(cJackClient, "frame_time", jr_client_get_frame_time, 0);
    rb_define_method(cJackClient, "last_frame_time", jr_client_get_last_frame_time, 0);
    rb_define_method(cJackClient, "frames_to_time", jr_client_frames_to_time, 1);
    rb_define_method(cJackClient, "time_to_frames", jr_client_time_to_frames, 1);
    rb_define_method(cJackClient, "time_to_frames", jr_client_time_to_frames, 1);

    rb_define_const(cJackClient, "NullOption", INT2NUM(JackNullOption));
    rb_define_const(cJackClient, "NoStartServer", INT2NUM(JackNoStartServer));
    rb_define_const(cJackClient, "UseExactName", INT2NUM(JackUseExactName));
    rb_define_const(cJackClient, "ServerName", INT2NUM(JackServerName));
    rb_define_const(cJackClient, "LoadName", INT2NUM(JackLoadName));
    rb_define_const(cJackClient, "LoadInit", INT2NUM(JackLoadInit));

    rb_define_const(cJackPort, "IsInput", INT2NUM(JackPortIsInput));
    rb_define_const(cJackPort, "IsOutput", INT2NUM(JackPortIsOutput));
    rb_define_const(cJackPort, "IsPhysical", INT2NUM(JackPortIsPhysical));
    rb_define_const(cJackPort, "CanMonitor", INT2NUM(JackPortCanMonitor));
    rb_define_const(cJackPort, "IsTerminal", INT2NUM(JackPortIsTerminal));
}
