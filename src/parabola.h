#ifndef PARABOLA_EXTRA_H
#define PARABOLA_EXTRA_H 1

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
inline double cs_parab_exec(double t, double n, double na);

#endif /* #ifndef PARABOLA_EXTRA_H */
