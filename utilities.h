/*

    SLIDEM Processor: utilities.h

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

#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdint.h>

#include <time.h>
#include <cdf.h>

#define UTC_DATE_LENGTH 24

// Constructs a full path to the export CDF file in the argument constructExportFileName.
int constructExportFileName(const char satellite, long year, long month, long day, const char *exportDir, double *beginTime, double *endTime, char *cdfFileName);

// Returns the number of records in the faceplate current CDF file.
long numberOfAvailableRecords(const char *ifpFilename);

void closeCdf(CDFid id);

// Prints an error message from the CDFstatus
void printErrorMessage(CDFstatus status);

void freeMemory(uint8_t **fpDataBuffers, uint8_t **hmDataBuffers, uint8_t **vnecDataBuffers, uint8_t **magDataBuffers, double *fpCurrent, double *vn, double *ve, double *vc, double *dipLat, double *dipLatitude, double *ionEffectiveMass, double *ionDensity, double *ionDriftRaw, double *ionDrift, double *ionEffectiveMassError, double *ionDensityError, double *ionDriftError, double *fpAreaOML, double *rProbeOML, double *electronTemperature, double *spacecraftPotential, double *faceplateVoltage, double *ionEffectiveMassTTS, uint32_t *mieffFlags, uint32_t *viFlags, uint32_t *niFlags, uint16_t *iterationCount);

int getInputFilename(const char satelliteLetter, long year, long month, long day, const char *path, const char *dataset, char *filename);

int dayOfYear(long year, long month, long day, int* yday);

void utcDateString(time_t seconds, char *dateString);

void utcDateStringWithMicroseconds(double seconds, char *dateString);

void utcNowDateString(char *dateString);

enum UTIL_ERRORS {
    UTIL_NO_ERROR = 0,
    UTIL_ERR_FP_FILENAME = -1,
    UTIL_ERR_HM_FILENAME = -2,
    UTIL_ERR_INPUT_FILE_MISMATCH = -3,
    UTIL_ERR_SATELLITE_LETTER = -4,
    UTIL_ERR_DAY_OF_YEAR_CONVERSION = -5
};

#endif // UTILITIES_H
