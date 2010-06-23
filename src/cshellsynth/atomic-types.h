#ifndef CSHELLSYNTH_ATOMIC_TYPES_H
#define CSHELLSYNTH_ATOMIC_TYPES_H 1

typedef struct {
	volatile int counter;
} atomic_t;

typedef struct {
	volatile float counter;
} atomic_float_t;

#ifdef __LP64__

typedef struct {
	volatile long counter;
} atomic64_t;

typedef atomic64_t atomic_ptr_t;
typedef atomic64_t atomic_long_t;

typedef struct {
	volatile double counter;
} atomic_double_t;

#else /* #ifdef __LP64__ */

typedef atomic_t atomic_ptr_t;
typedef atomic_t atomic_long_t;

typedef struct {
	volatile int counter;
	volatile double d;
} atomic_double_t;

#endif /* #ifdef __LP64__ */

#endif /* #ifndef CSHELLSYNTH_ATOMIC_TYPES_H */
