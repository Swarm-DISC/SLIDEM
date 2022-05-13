/*

    SLIDEM Processor: post_process_ion_drift.h

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

#ifndef _POST_PROCESS_ION_DRIFT_H
#define _POST_PROCESS_ION_DRIFT_H

#include <stdint.h>
#include <stdio.h>

// After EFI TCT processor
// Background IP

typedef struct offset_model_fit_arguments {
    const uint8_t regionNumber;
    const char *regionName;
    const float lat1;
    const float lat2;
    const float lat3;
    const float lat4;
} offset_model_fit_arguments;


void postProcessIonDrift(const char *slidemFilename, const char satellite, uint8_t **hmDataBuffers, double *fpCurrent, double *ionDrift, double *ionDriftError, uint32_t *viFlags, long nHmRecs);

void removeOffsetsAndSetFlags(const char satellite, offset_model_fit_arguments fitargs, long nRecs, uint8_t **hmDataBuffers, double *fpCurrent, double *ionDrift, double *ionDriftError, uint32_t *viFlags, FILE* fitFile);





#endif // _POST_PROCESS_ION_DRIFT_H