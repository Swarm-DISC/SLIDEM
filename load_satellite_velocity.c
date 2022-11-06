/*

    SLIDEM Processor: load_satellite_velocity.c

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

#include "load_satellite_velocity.h"

#include "slidem_settings.h"
#include "utilities.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <cdf.h>

extern char infoHeader[50];

int loadSatelliteVelocity(const char *modFilename, uint8_t **vnecDataBuffers, long *nVnecRecs)
{
    int status = (int) SAT_VEL_ERROR_UNAVAILABLE;

    FILE *modFP = fopen(modFilename, "r");
    if (modFP == NULL)
    {
        return SAT_VEL_ERROR_FILE;
    }

    char buf[100] = {0};

    int year;
    int month;
    int day;
    int hour;
    int minute;
    double seconds;
    int sec;
    int msec;

    double x, y, z;
    double vx, vy, vz;
    double dummy;

    double cdfTime;

    char * str = NULL;
    int itemsConverted = 0;
    long records = 0;

    double cx, cy, cz, ex, ey, ez, nx, ny, nz;
    double cm, em, nm;
    double vn, ve, vc;

    long epochs = 0;
    int a, b, c, d;
    double e;

    itemsConverted = fscanf(modFP, "%3c%d %d %d %d %d %lf %ld %5c %5c %3c %4c\n", buf, &year, &month, &day, &hour, &minute, &seconds, &epochs, buf, buf, buf, buf);

    if (epochs < MINIMUM_VELOCITY_EPOCHS)
    {
        status = SAT_VEL_ERROR_TOO_FEW_EPOCHS;
        goto cleanup;
    }

    for (int i = 0; i < 4; i++)
    {
        vnecDataBuffers[i] = (uint8_t*) malloc((size_t) (epochs * sizeof(double)));
        if (vnecDataBuffers[i] == NULL)
        {
            for (int j = 0; j < i; j++)
            {
                free(vnecDataBuffers[j]);
            }
            status = SAT_VEL_ERROR_MEMORY;
            goto cleanup;
        }
    }

    long lines = 1;

    sec = (int)floor(seconds);
    msec = (int)floor(1000.0 * (seconds - (double)sec));
    double gpsEpoch = computeEPOCH(year, month, day, hour, minute, sec, msec);
    double utEpoch = computeEPOCH(year, month, day, 0, 0, 0, 0);
    double gpsTimeOffset = gpsEpoch - utEpoch;
    double gpsTime = 0.0;

    while(fgets(buf, 100, modFP) != NULL)
    {
        lines++;
        if (buf[0] != '*')
        {
            continue;
        }
        sscanf(buf+2, "%d %d %d %d %d %lf", &year, &month, &day, &hour, &minute, &seconds);
        sec = (int) floor(seconds);
        msec = 1000 * (int)floor(seconds - (double)sec);
        gpsTime = computeEPOCH(year, month, day, hour, minute, sec, msec);
        cdfTime = gpsTime - gpsTimeOffset;
        if(fgets(buf, 100, modFP) == NULL || buf[0] != 'P')
        {
            status = SAT_VEL_ERROR_FILE;
            goto cleanup;
        }
        lines++;
        sscanf(buf+5, "%lf %lf %lf", &x, &y, &z);
        if(fgets(buf, 100, modFP) == NULL || buf[0] != 'V')
        {
            status = SAT_VEL_ERROR_FILE;
            goto cleanup;
        }
        sscanf(buf+5, "%lf %lf %lf", &vx, &vy, &vz);
        lines++;
        records++;
        vx /= 10.;
        vy /= 10.;
        vz /= 10.;

        // Calculate Vnec
        // chat
        cx = -x; cy = -y; cz = -z;
        cm = sqrt(cx*cx + cy*cy + cz*cz);
        cx /= cm; cy /= cm; cz /= cm;
        // ehat
        ex = cy; ey = -cx; ez = 0.0;
        em = sqrt(ex*ex + ey*ey + ez*ez);
        ex /= em; ey /= em;
        // nhat
        nx = -cx * cz; ny = -cy * cz; nz = cx*cx + cy*cy;
        nm = sqrt(nx*nx + ny*ny + nz*nz);
        nx /= nm; ny /= nm; nz /= nm;

        // vnec
        vn = vx * nx + vy * ny + vz * nz;
        ve = vx * ex + vy * ey + vz * ez;
        vc = vx * cx + vy * cy + vz * cz;

        ((double*)vnecDataBuffers[0])[records-1] = cdfTime;
        ((double*)vnecDataBuffers[1])[records-1] = vn;
        ((double*)vnecDataBuffers[2])[records-1] = ve;
        ((double*)vnecDataBuffers[3])[records-1] = vc;

    }

    if (records != epochs)
    {
        status = SAT_VEL_ERROR_WRONG_NUMBER_OF_RECORDS_READ;
        goto cleanup;
    }

    *nVnecRecs = records;
    status = SAT_VEL_OK;

cleanup:

    fclose(modFP);
    return status;

}

