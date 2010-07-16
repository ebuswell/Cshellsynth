/*
 * atomic-types.h
 * 
 * Copyright 2010 Evan Buswell
 * 
 * This file is part of Cshellsynth.
 * 
 * Cshellsynth is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Cshellsynth is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Cshellsynth.  If not, see <http://www.gnu.org/licenses/>.
 */
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
