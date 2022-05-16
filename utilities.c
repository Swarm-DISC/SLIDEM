/*

    SLIDEM Processor: utilities.c

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

// Adapted from the Swarm Thermal Ion Imager Cross-track Ion Drift processor source code

#include "utilities.h"
#include "slidem_settings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include <fts.h>
#include <math.h>


// Prefix for all fprintf messages
extern char infoHeader[50];


// Generates the filename for exported CDF file, with full path
int constructExportFileName(const char satellite, long year, long month, long day, const char *exportDir, double *beginTime, double *endTime, char *cdfFileName)
{

    if (satellite != 'A' && satellite != 'B' && satellite != 'C')
    {
        return UTIL_ERR_SATELLITE_LETTER;
    }

    *beginTime = computeEPOCH(year, month, day, 0, 0, 0, 0);
    *endTime = computeEPOCH(year, month, day, 23, 59, 59, 999);

    sprintf(cdfFileName, "%s/SW_%s_EFI%c%s_2__%04d%02d%02dT000000_%04d%02d%02dT235959_%s", exportDir, SLIDEM_PRODUCT_TYPE, satellite, SLIDEM_PRODUCT_CODE, (int)year, (int)month, (int)day, (int)year, (int)month, (int)day, EXPORT_VERSION_STRING);

    return UTIL_NO_ERROR;

}

long numberOfAvailableRecords(const char *fpFilename)
{
    CDFsetValidate(VALIDATEFILEoff);
    CDFid calCdfId;
    CDFstatus status;
    status = CDFopenCDF(fpFilename, &calCdfId);
    if (status != CDF_OK) 
    {
        // Not necessarily an error. For example, some dates will have not calibration data.
        printErrorMessage(status);
        return 0;
    }

    // Get number of records for zVar "epoch"
    long nRecords = 0;
    status = CDFgetzVarAllocRecords(calCdfId, CDFgetVarNum(calCdfId, "Timestamp"), &nRecords);
    if (status != CDF_OK) 
    {
        printErrorMessage(status);
        nRecords = 0;
    }
    closeCdf(calCdfId);
    return nRecords;

}

void closeCdf(CDFid id)
{
    CDFstatus status;
    status = CDFcloseCDF(id);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
    }

}

void printErrorMessage(CDFstatus status)
{
    char errorMessage[CDF_STATUSTEXT_LEN + 1];
    CDFgetStatusText(status, errorMessage);
    fprintf(stdout, "%s%s\n", infoHeader, errorMessage);
}

void freeMemory(uint8_t **fpDataBuffers, uint8_t **hmDataBuffers, uint8_t **vnecDataBuffers, uint8_t **magDataBuffers, double *fpCurrent, double *vn, double *ve, double *vc, double *dipLat, double *dipLatitude, double *ionEffectiveMass, double *ionDensity, double *ionDriftRaw, double *ionDrift, double *ionEffectiveMassError, double *ionDensityError, double *ionDriftError, double *fpAreaOML, double *rProbeOML, double *electronTemperature, double *spacecraftPotential, double *faceplateVoltage, double *ionEffectiveMassTTS, uint32_t *mieffFlags, uint32_t *viFlags, uint32_t *niFlags, uint16_t *iterationCount)
{
    for (uint8_t i = 0; i < NUM_FP_VARIABLES; i++)
    {
        free(fpDataBuffers[i]);
    }
    for (uint8_t i = 0; i < NUM_HM_VARIABLES; i++)
    {
        free(hmDataBuffers[i]);
    }
    for (uint8_t i = 0; i < NUM_VNEC_VARIABLES; i++)
    {
        free(vnecDataBuffers[i]);
    }
    for (uint8_t i = 0; i < NUM_MAG_VARIABLES; i++)
    {
        free(magDataBuffers[i]);
    }
    free(fpCurrent);
    free(vn);
    free(ve);
    free(vc);
    free(dipLat); // 1 Hz
    free(dipLatitude); // 2 Hz
    free(ionEffectiveMass);
    free(ionDensity);
    free(ionDriftRaw);
    free(ionDrift);
    free(ionEffectiveMassError);
    free(ionDensityError);
    free(ionDriftError);
    free(fpAreaOML);
    free(rProbeOML);
    free(electronTemperature);
    free(spacecraftPotential);
    free(faceplateVoltage);
    free(ionEffectiveMassTTS);
    free(mieffFlags);
    free(viFlags);
    free(niFlags);
    free(iterationCount);
}


int getInputFilename(const char satelliteLetter, long year, long month, long day, const char *path, const char *dataset, char *filename)
{
	char *searchPath[2] = {NULL, NULL};
    searchPath[0] = (char *)path;

	FTS * fts = fts_open(searchPath, FTS_PHYSICAL | FTS_NOCHDIR, NULL);	
	if (fts == NULL)
	{
		printf("Could not open directory %s for reading.", path);
		return UTIL_ERR_HM_FILENAME;
	}
	FTSENT * f = fts_read(fts);

    bool gotHmFile = false;
    long fileYear;
    long fileMonth;
    long fileDay;
    long lastVersion = -1;
    long fileVersion;
	while(f != NULL)
	{
        // Most Swarm CDF file names have a length of 59 characters. The MDR_MAG_LR files have a lend of 70 characters.
        // The MDR_MAG_LR files have the same filename structure up to character 55.
		if ((strlen(f->fts_name) == 59 || strlen(f->fts_name) == 70) && *(f->fts_name+11) == satelliteLetter && strncmp(f->fts_name+13, dataset, 5) == 0)
		{
            char fyear[5] = { 0 };
            char fmonth[3] = { 0 };
            char fday[3] = { 0 };
            char version[5] = { 0 };
            strncpy(fyear, f->fts_name + 19, 4);
            fileYear = atol(fyear);
            strncpy(fmonth, f->fts_name + 23, 2);
            fileMonth = atol(fmonth);
            strncpy(fday, f->fts_name + 25, 2);
            fileDay = atol(fday);
            strncpy(version, f->fts_name + 51, 4);
            fileVersion = atol(version);
            if (fileYear == year && fileMonth == month && fileDay == day && fileVersion > lastVersion)
            {
                lastVersion = fileVersion;
                sprintf(filename, "%s", f->fts_path);
                gotHmFile = true;
            }
		}
		f = fts_read(fts);
	}

	fts_close(fts);

    if (gotHmFile)
    {
        return 0;
    }
    else
    {
        return UTIL_ERR_HM_FILENAME;
    }

}

// Calculates day of year: 1 January is day 1.
int dayOfYear(long year, long month, long day, int* yday)
{
    time_t date;
    struct tm dateStruct;
    dateStruct.tm_year = year - 1900;
    dateStruct.tm_mon = month - 1;
    dateStruct.tm_mday = day;
    dateStruct.tm_hour = 0;
    dateStruct.tm_min = 0;
    dateStruct.tm_sec = 0;
    dateStruct.tm_yday = 0;
    date = timegm(&dateStruct);
    struct tm *dateStructUpdated = gmtime(&date);
    if (dateStructUpdated == NULL)
    {
        fprintf(stdout, "%sUnable to get day of year from specified date.\n", infoHeader);
        *yday = 0;
        return UTIL_ERR_DAY_OF_YEAR_CONVERSION;
    }
    *yday = dateStructUpdated->tm_yday + 1;
    return UTIL_NO_ERROR;
}

void utcDateString(time_t seconds, char *dateString)
{
    struct tm *d = gmtime(&seconds);
    sprintf(dateString, "UTC=%04d-%02d-%02dT%02d:%02d:%02d", d->tm_year + 1900, d->tm_mon + 1, d->tm_mday, d->tm_hour, d->tm_min, d->tm_sec);

    return;
}

void utcNowDateString(char *dateString)
{
    time_t seconds = time(NULL);
    struct tm *d = gmtime(&seconds);
    sprintf(dateString, "UTC=%04d-%02d-%02dT%02d:%02d:%02d", d->tm_year + 1900, d->tm_mon + 1, d->tm_mday, d->tm_hour, d->tm_min, d->tm_sec);

    return;
}

void utcDateStringWithMicroseconds(double exactSeconds, char *dateString)
{
    time_t seconds = (time_t) floor(exactSeconds);
    int microseconds = (int) floor(1000000.0 * (exactSeconds - (double) seconds));
    struct tm *d = gmtime(&seconds);
    sprintf(dateString, "UTC=%04d-%02d-%02dT%02d:%02d:%02d.%06d", d->tm_year + 1900, d->tm_mon + 1, d->tm_mday, d->tm_hour, d->tm_min, d->tm_sec, microseconds);

    return;

}