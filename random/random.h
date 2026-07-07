/**
 * ============================================================================================
 * Random library
 * ============================================================================================
 * A compact library with methods for random number generation.
 * ============================================================================================
 * License and Copyright
 * ============================================================================================
 * Copyright (c) 2026 github.com/b0-b
 * This program is free software: you can redistribute it and/or modify it under the terms of 
 * the GNU General Public License as published by the Free Software Foundation, either version 
 * 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details. You should have received a copy of the 
 * GNU General Public License along with this program. If not, see <https://gnu.org>.
 */

#include <math.h>
#include <Arduino.h>

/**
 * Generates a random number uniformly from the provided interval [a, b].
 */
inline long uniform (long a, long b) {
  // Makes the range [a, b] fully inclusive using pure integer math
  return random(a, b + 1);
}

/**
 * Generates an independent and identically distributed float between 0 and 1. 
 * Alias for uniform(0, 1). 
 */
inline double u () {
    // Divides a random number by the maximum possible random number
    return (double)random(0, LONG_MAX) / LONG_MAX;
}

/**
 * @brief Generates a pseudo-random float from a normal distribution.
 * 
 * Uses the Box-Muller transform method to generate a standard normal 
 * variable, then scales and shifts it to the requested distribution.
 * 
 * @param mean  The desired mean (mu) of the normal distribution. Default: 0
 * @param sigma The standard deviation (sigma) of the normal distribution. Default: 1
 * @return      A random float sample fitting N(mean, sigma^2).
 * 
 * @note Requires the helper function iid() to return a uniform distribution 
 *       strictly in the range (0, 1] to avoid a log-of-zero error.
 */
float norm (float mean=0, float sigma=1) {
    const float u1 = (float)u();
    const float u2 = (float)u();
    // Use 1-D box muller method to generate a standard normal ~ N(0, 1)
    float stdNorm;
    if (u() > 0.5)
        stdNorm = sqrtf(-2*logf(u1)) * cosf( 2.0f * PI * u2 );
    else
        stdNorm = sqrtf(-2*logf(u1)) * sinf( 2.0f * PI * u2 );

    // First scale up and shift to obtain desired mean and sigma
    return mean + stdNorm * sigma;
}