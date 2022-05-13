/*

    SLIDEM Processor: interpolate.h

    Copyright (C) 2022  Johnathan K Burchill

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _INTERPOLATE_H
#define _INTERPOLATE_H

#include <stdint.h>
#include <gsl/gsl_interp.h>
#include <gsl/gsl_spline.h>

void interpolateFpCurrent(uint8_t **fpDataBuffers, long nFpRecs, uint8_t **hmDataBuffers, long nHmRecs, double interpolates[]);

void interpolateVNEC(uint8_t **vnecDataBuffers, long nVnecRecs, uint8_t **hmDataBuffers, long nHmRecs, double interpolates[], int vnecIndex);

void interpolateDipLatitude(double *timeIn, double * dipLatIn, long nDipLatRecs, uint8_t **hmDataBuffers, long nHmRecs, double interpolates[]);

#endif // _INTERPOLATE_H
