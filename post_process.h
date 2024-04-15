/*

    SLIDEM Processor: post_process.h

    Copyright (C) 2024 Johnathan K Burchill

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

#ifndef _POST_PROCESS_H
#define _POST_PROCESS_H

#include <stdint.h>
#include <stdio.h>

#include "modified_oml.h"

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


void postProcessIonDrift(const char *slidemFilename, const char satellite, uint8_t **hmDataBuffers, double *vn, double *ve, double *vc, double *dipLatitude, double *fpCurrent, double *faceplateVoltage, double *fpAreaOML, double *rProbeOML, double *electronTemperature, double *spacecraftPotential, uint32_t *electronTemperatureSource, uint32_t *spacecraftPotentialSource, double *ionEffectiveMassTTS, double *ionDrift, double *ionDriftError, double *ionEffectiveMass, double *ionEffectiveMassError, double *ionDensity, double *ionDensityError, uint32_t *viFlags, uint32_t *mieffFlags, uint32_t *niFlags, uint16_t *iterationCount, probeParams sphericalProbeParams, long nHmRecs);

void removeOffsetsAndSetFlags(const char satellite, offset_model_fit_arguments fitargs, long nHmRecs, uint8_t **hmDataBuffers, double *vn, double *ve, double *vc, double *dipLatitude, double *fpCurrent, double *faceplateVoltage, double *fpAreaOML, double *rProbeOML, double *electronTemperature, double *spacecraftPotential, uint32_t *electronTemperatureSource, uint32_t *spacecraftPotentialSource, double *ionEffectiveMassTTS, double *ionDrift, double *ionDriftError, double *ionEffectiveMass, double *ionEffectiveMassError, double *ionDensity, double *ionDensityError, uint32_t *viFlags, uint32_t *mieffFlags, uint32_t *niFlags, uint16_t *iterationCount, probeParams sphericalProbeParams, FILE* fitFile);





#endif // _POST_PROCESS_H
