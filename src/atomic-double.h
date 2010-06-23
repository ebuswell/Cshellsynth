#ifndef _ASM_X86_ATOMIC_DOUBLE_H
#define _ASM_X86_ATOMIC_DOUBLE_H

#include "atomic.h"
#include "cshellsynth/atomic-types.h"

/*
 * Atomic operations that C can't guarantee us.  Useful for
 * resource counting etc..
 */

#define ATOMIC_DOUBLE_INIT(d)	{ (d) }

/**
 * atomic_double_read - read atomic variable
 * @v: pointer of type atomic_double_t
 *
 * Atomically reads the value of @v.
 */
static inline double atomic_double_read(const atomic_double_t *v)
{
#ifdef __LP64__
	return v->counter;
#else
	return v->d;
#endif
}

/**
 * atomic_double_set - set atomic variable
 * @v: pointer of type atomic_double_t
 * @f: required value
 *
 * Atomically sets the value of @v to @f.
 */
static inline void atomic_double_set(atomic_double_t *v, double d)
{
#ifdef __LP64__
	v->counter = d;
#else
	v->d = d;
#endif
}

#ifdef __LP64__

static inline double atomic_double_cmpxchg(atomic_double_t *v, double old, double new)
{
	union {
		double d;
		long l;
	} u_old;
	union {
		double d;
		long l;
	} u_new;
	union {
		double d;
		long l;
	} r;
	u_old.d = old;
	u_new.d = new;
	r.l = atomic64_cmpxchg((atomic64_t *) v, u_old.l, u_new.l);
	return r.d;
}

static inline double atomic_double_xchg(atomic_double_t *v, double new)
{
	union {
		double d;
		long l;
	} u_new;
	union {
		double d;
		long l;
	} r;
	u_new.d = new;
	r.l = atomic64_xchg((atomic64_t *) v, new);

	return r.d;
}

#else /* __LP64__ */

static inline double atomic_double_cmpxchg(atomic_double_t *v, double old, double new)
{
	double prev;
	// spinlock
	while(unlikely(atomic_cmpxchg((atomic_t) v, 0, 1)));
	prev = v->d;
	if(likely(prev == old))
		v->d = new;
	atomic_set((atomic_t) v, 0);
	return prev;
}

static inline double atomic_double_xchg(atomic_double_t *v, double new)
{
	double prev;
	// spinlock
	while(unlikely(atomic_cmpxchg((atomic_t) v, 0, 1)));
	prev = v->d;
	v->d = new;
	atomic_set((atomic_t) v, 0);
	return prev;
}

#endif /* __LP64__ */

/**
 * atomic_double_add - add integer to atomic variable
 * @d: double value to add
 * @v: pointer of type atomic_double_t
 *
 * Atomically adds @d to @v.
 */
static inline void atomic_double_add(double d, atomic_double_t *v)
{
	double old;
	double c;
	c = atomic_double_read(v);
	for(;;) {
		old = atomic_double_cmpxchg((v), c, c + (d));
		if(likely(old == c))
			break;
		c = old;
	}
}

/**
 * atomic_double_sub - subtract the atomic variable
 * @d: double value to subtract
 * @v: pointer of type atomic_double_t
 *
 * Atomically subtracts @d from @v.
 */
static inline void atomic_double_sub(double d, atomic_double_t *v)
{
	double old;
	double c;
	c = atomic_double_read(v);
	for(;;) {
		old = atomic_double_cmpxchg(v, c, c - d);
		if(likely(old == c))
			break;
		c = old;
	}
}

/**
 * atomic_double_add_return - add and return
 * @d: double value to add
 * @v: pointer of type atomic_double_t
 *
 * Atomically adds @d to @v and returns @d + @v
 */
static inline double atomic_double_add_return(double d, atomic_double_t *v)
{
	double old;
	double new;
	double c;
	c = atomic_double_read(v);
	for(;;) {
		new = c + d;
		old = atomic_double_cmpxchg(v, c, new);
		if(likely(old == c))
			break;
		c = old;
	}
	return new;
}

static inline double atomic_double_sub_return(double d, atomic_double_t *v)
{
	double old;
	double new;
	double c;
	c = atomic_double_read(v);
	for(;;) {
		new = c - d;
		old = atomic_double_cmpxchg(v, c, new);
		if(likely(old == c))
			break;
		c = old;
	}
	return new;
}

/**
 * atomic_double_sub_and_test - subtract value from variable and test result
 * @d: double value to subtract
 * @v: pointer of type atomic_double_t
 *
 * Atomically subtracts @d from @v and returns
 * true if the result is zero, or false for all
 * other cases.
 */
static inline int atomic_double_sub_and_test(double d, atomic_double_t *v)
{
	return atomic_double_sub_return(d, v) == 0.0;
}

/**
 * atomic_double_inc - increment atomic variable
 * @v: pointer of type atomic_double_t
 *
 * Atomically increments @v by 1.
 */
static inline void atomic_double_inc(atomic_double_t *v)
{
	atomic_double_add(1.0, v);
}

/**
 * atomic_double_dec - decrement atomic variable
 * @v: pointer of type atomic_double_t
 *
 * Atomically decrements @v by 1.
 */
static inline void atomic_double_dec(atomic_double_t *v)
{
	atomic_double_sub(1.0, v);
}

/**
 * atomic_double_dec_and_test - decrement and test
 * @v: pointer of type atomic_double_t
 *
 * Atomically decrements @v by 1 and
 * returns true if the result is 0, or false for all other
 * cases.
 */
static inline int atomic_double_dec_and_test(atomic_double_t *v)
{
	return atomic_double_sub_and_test(1.0, v);
}

/**
 * atomic_double_inc_and_test - increment and test
 * @v: pointer of type atomic_double_t
 *
 * Atomically increments @v by 1
 * and returns true if the result is zero, or false for all
 * other cases.
 */
static inline int atomic_double_inc_and_test(atomic_double_t *v)
{
	return atomic_double_add_return(1.0, v) == 0.0;
}

/**
 * atomic_double_add_negative - add and test if negative
 * @d: double value to add
 * @v: pointer of type atomic_double_t
 *
 * Atomically adds @i to @v and returns true
 * if the result is negative, or false when
 * result is greater than or equal to zero.
 */
static inline int atomic_double_add_negative(double d, atomic_double_t *v)
{
	return atomic_double_add_return(d, v) < 0.0;
}

#define atomic_double_inc_return(v)  (atomic_double_add_return(1.0, v))
#define atomic_double_dec_return(v)  (atomic_double_sub_return(1.0, v))


/**
 * atomic_double_add_unless - add unless the number is a given value
 * @v: pointer of type atomic_double_t
 * @a: the amount to add to v...
 * @u: ...unless v is equal to u.
 *
 * Atomically adds @a to @v, so long as it was not @u.
 * Returns non-zero if @v was not @u, and zero otherwise.
 */
static inline int atomic_double_add_unless(atomic_double_t *v, double a, double u)
{
	double c, old;
	c = atomic_double_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic_double_cmpxchg((v), c, c + (a));
		if (likely(old == c))
			break;
		c = old;
	}
	return c != (u);
}

#define atomic_double_inc_not_zero(v) atomic_double_add_unless((v), 1.0, 0.0)

#endif /* _ASM_X86_ATOMIC_DOUBLE_64_H */
