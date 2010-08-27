/*
 * falling_saw.h
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
#ifndef FALLING_SAW_EXTRA_H
#define FALLING_SAW_EXTRA_H 1

#include <math.h>
#include "util.h"

/**
 * Calculate the bandlimited sawtooth for the given parameters:
 *
 * @param t offset
 * @param n number of harmonics
 * @param na fraction of the last harmonic
 *
 * See @ref cs_synth_destroy
 */
inline double cs_fsaw_exec(double t, double n, double na);

#endif /* #ifndef FALLING_SAW_EXTRA_H */
