#include <jack/jack.h>
#include <math.h>
#include "cshellsynth/ampeg_vt22.h"
#include "cshellsynth/filter.h"
#include "atomic-float.h"

/*
      o v
      |
      \
      / a
      \
      |--o y
     _|_
x o--_-_-
      |
      |--o z
      |
      \
    b /
      \
   o--|
      \
    c /
      \
     _|_
      -

ik = ((x - z) + (y - z)/u)^(3/2)
ik = ((x - ib) + (y - i(b + c))/u)^(3/2)
ik = ((x - ib) + (v - ia - i(b + c))/u)^(3/2)
ik = ((x - ib) + (v - i(a + b + c))/u)^(3/2)
ik = (x - ib + v/u - i(a + b + c)/u)^(3/2)
ik = (x + v/u - i(b + (a + b + c)/u))^(3/2)
let m/3 = x + v/u
let p = (b + (a + b + c)/u)
ik = (m/3 - ip)^(3/2)
let n = ip, i = n/p
nk/p = (m/3 - n)^(3/2)
n^2(k/p)^2 = (m/3 - n)^3
let q = (k/p)^2
n^2q = (m/3 - n)^3

E = ((m*sqrt(m*(9*m-4*q))-(2*q^2-6*m*q+3*m^2))*q/54)^(1/3)
n = E-(2*m*q-q^2)/(9*E)+(m-q)/3
--------------------
let p = (b + (a + b + c)/u)
let q = (k/p)^2
let n = ip, i = n/p
let m/3 = x + v/u, m = 3(x + v/u)

*/

#define process_stage(a, b, c, v, u, k, x) process_stage_f((b + (a + b + c)/u), (k/(b + (a + b + c)/u))*(k/(b + (a + b + c)/u)), 3*(x) + (v)/u)

static inline double process_stage_f(double p, double q, double m) {
    double E = powf(((m*sqrt(m*(9.0*m-4.0*q))-(2.0*q*q-6*m*q+3*m*m))*(q/54.0)), (1/3));
    double n = E-((2*m-q)*q)/(9*E)+(m-q)/3;
    return n/p;
}

#define i_to_y(a, b, c, v, i) ((v) - (i)*(a))
#define i_to_z(a, b, c, v, i) (i)*((b) + (c))

#define max_i(a, b, c, v) (v)/((a) + (b) + (c))

double stage_1_zero = 0.0;
double stage_2_zero = 0.0;
double stage_3a_zero = 0.0;
double stage_3b_zero = 0.0;
double stage_3_zero_feedback = 0.0;
double stage_4_zero = 0.0;

static inline double stage_1(double x) {
    if(stage_1_zero == 0.0) {
	double i = process_stage(390000.0, 6800.0, 0.0, 325.0, 100.0, 1060.0, 0.0);
	if(i < 0.0 || isnan(i)) i = 0.0;
	if(i > max_i(390000.0, 6800.0, 0.0, 325.0))
	    i = max_i(390000.0, 6800.0, 0.0, 325.0);
	stage_1_zero = i_to_y(390000.0, 6800.0, 0.0, 325.0, i);
    }
    double i = process_stage(390000.0, 6800.0, 0.0, 325.0, 100.0, 1060.0, x);
    if(i < 0.0 || isnan(i)) i = 0.0;
    if(i > max_i(390000.0, 6800.0, 0.0, 325.0))
	i = max_i(390000.0, 6800.0, 0.0, 325.0);
    return i_to_y(390000.0, 6800.0, 0.0, 325.0, i) - stage_1_zero;
}

static inline double stage_2(double x) {
    if(stage_2_zero == 0.0) {
	double i = process_stage(68000.0, 3300.0, 0.0, 325.0, 1060.0, 100.0, 0.0);
	if(i < 0.0 || isnan(i)) i = 0.0;
	if(i > max_i(68000.0, 3300.0, 0.0, 325.0))
	    i = max_i(68000.0, 3300.0, 0.0, 325.0);
	stage_2_zero = i_to_y(68000.0, 3300.0, 0.0, 325.0, i);
    }
    double i = process_stage(68000.0, 3300.0, 0.0, 325.0, 1060.0, 100.0, x);
    if(i < 0.0 || isnan(i)) i = 0.0;
    if(i > max_i(68000.0, 3300.0, 0.0, 325.0))
	i = max_i(68000.0, 3300.0, 0.0, 325.0);
    return i_to_y(68000.0, 3300.0, 0.0, 325.0, i) - stage_2_zero;
}

static inline double stage_3(double x) {
    if(stage_3a_zero == 0.0) {
	double i = process_stage(470000.0, 3300.0, 0.0, 325.0, 1060.0, 100.0, 0.0);
	if(i < 0.0 || isnan(i)) i = 0.0;
	if(i > max_i(470000.0, 3300.0, 0.0, 325.0))
	    i = max_i(470000.0, 3300.0, 0.0, 325.0);
	double x2 =  i_to_y(470000.0, 3300.0, 0.0, 325.0, i);
	i = process_stage(0.0, (29241.710504), 0.0, 325.0, 1180.0, 17, x2);
	if(i < 0.0 || isnan(i)) i = 0.0;
	if(i > max_i(0.0, (29241.710504), 0.0, 325.0))
	    i = max_i(0.0, (29241.710504), 0.0, 325.0);
	stage_3_zero_feedback = 0.543526217546 * i * (7500.0 + 560.0);
	stage_3b_zero = i*0.456473782454*6800.0;

	i = process_stage(220000.0, 560.0, 7500.0, 325.0 - stage_3_zero_feedback, 1060.0, 100.0, 0.0);
	if(i < 0.0 || isnan(i)) i = 0.0;
	if(i > max_i(220000.0, 560.0, 7500.0, 325.0))
	    i = max_i(220000.0, 560.0, 7500.0, 325.0);
	stage_3a_zero =  i_to_y(220000.0, 560.0, 7500.0, 325.0, i);

    }
    double feedback = stage_3_zero_feedback;
    
    double i = process_stage(220000.0, 560.0, 7500.0, 325.0 - feedback, 1060.0, 100.0, x);
    if(i < 0.0 || isnan(i)) i = 0.0;
    if(i > max_i(220000.0, 560.0, 7500.0, 325.0))
	i = max_i(220000.0, 560.0, 7500.0, 325.0);
    double x2 =  i_to_y(220000.0, 560.0, 7500.0, 325.0, i) - stage_3a_zero;
    i = process_stage(470000.0, 3300.0, 0.0, 325.0, 1060.0, 100.0, x);
    if(i < 0.0 || isnan(i)) i = 0.0;
    if(i > max_i(470000.0, 3300.0, 0.0, 325.0))
	i = max_i(470000.0, 3300.0, 0.0, 325.0);
    x2 =  i_to_y(470000.0, 3300.0, 0.0, 325.0, i);
    i = process_stage(0.0, (29241.710504), 0.0, 325.0, 1180.0, 17, x2);
    if(i < 0.0 || isnan(i)) i = 0.0;
    if(i > max_i(0.0, (29241.710504), 0.0, 325.0))
	i = max_i(0.0, (29241.710504), 0.0, 325.0);
    double old_i;

    do {
	feedback = 0.543526217546 * i * (7500.0 + 560.0);
	old_i = i;
	i = process_stage(220000.0, 560.0, 7500.0, 325.0 - feedback, 1060.0, 100.0, x);
	if(i < 0.0 || isnan(i)) i = 0.0;
	if(i > max_i(220000.0, 560.0, 7500.0, 325.0))
	    i = max_i(220000.0, 560.0, 7500.0, 325.0);
	x2 =  i_to_y(220000.0, 560.0, 7500.0, 325.0, i) - stage_3a_zero;
	i = process_stage(470000.0, 3300.0, 0.0, 325.0, 1060.0, 100.0, x);
	if(i < 0.0 || isnan(i)) i = 0.0;
	if(i > max_i(470000.0, 3300.0, 0.0, 325.0))
	    i = max_i(470000.0, 3300.0, 0.0, 325.0);
	x2 =  i_to_y(470000.0, 3300.0, 0.0, 325.0, i);
	i = process_stage(0.0, (29241.710504), 0.0, 325.0, 1180.0, 17, x2);
	if(i < 0.0 || isnan(i)) i = 0.0;
	if(i > max_i(0.0, (29241.710504), 0.0, 325.0))
	    i = max_i(0.0, (29241.710504), 0.0, 325.0);
    } while(abs(i - old_i)/i < 0.001);

    return (i*0.456473782454*6800.0 - stage_3b_zero)*(1000000.0/(180000.0 + 1000000.0));
}

static inline double stage_4(double x) {
    if(stage_4_zero == 0.0) {
	double i = process_stage(0.0, 1000.0, 47000.0, 325.0, 1180.0, 20, 0);
	if(i < 0.0 || isnan(i)) i = 0.0;
	if(i > max_i(0.0, 1000.0, 47000.0, 325.0))
	    i = max_i(0.0, 1000.0, 47000.0, 325.0);
	stage_4_zero = i_to_z(0.0, 1000.0, 47000.0, 325.0, i);
    }
    double i = process_stage(0.0, 1000.0, 47000.0, 325.0, 1180.0, 20, x);
    if(i < 0.0 || isnan(i)) i = 0.0;
    if(i > max_i(0.0, 1000.0, 47000.0, 325.0))
	i = max_i(0.0, 1000.0, 47000.0, 325.0);
    return (i_to_z(0.0, 1000.0, 47000.0, 325.0, i) - stage_4_zero) * 100000.0/(10000.0 + 100000.0);
}

static int cs_ampeg_vt22_process(jack_nframes_t nframes, void *arg) {
    cs_ampeg_vt22_t *self = (cs_ampeg_vt22_t *) arg;
    float *in_buffer;
    float *out_buffer = (float *)jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    float in = atomic_float_read(&self->in);
    if(isnanf(in)) {
	in_buffer = (float *)jack_port_get_buffer(self->in_port, nframes);
	if(in_buffer == NULL) {
	    return -1;
	}
    }
    float gain = atomic_float_read(&self->gain);
    int i;
    for(i = 0; i < nframes; i++) {
	float f = stage_1(0.05*(isnanf(in) ? in_buffer[i] : in));
	f *= gain;
	f = stage_2(f);
	f = stage_3(f);
	f = stage_4(f);
	f *= (1.0f/250.0f);
	out_buffer[i] =  f;
    }
    return 0;
}

void cs_ampeg_vt22_set_gain(cs_ampeg_vt22_t *self, float gain) {
    atomic_float_set(&self->gain, gain);
}

int cs_ampeg_vt22_init(cs_ampeg_vt22_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_filter_init((cs_filter_t *) self, client_name, flags, server_name);
    if(r != 0) {
	return r;
    }

    atomic_float_set(&self->gain, 0.125f);

    r = jack_set_process_callback(self->client, cs_ampeg_vt22_process, self);
    if(r != 0) {
	cs_filter_destroy((cs_filter_t *) self);
	return r;
    }

    r = jack_activate(self->client);
    if(r != 0) {
	cs_filter_destroy((cs_filter_t *) self);
	return r;
    }

    return 0;
}
