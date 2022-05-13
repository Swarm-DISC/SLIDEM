/*

    SLIDEM Processor: interpolate.c

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

#include "interpolate.h"
#include "main.h"
#include "slidem_settings.h"

#include <math.h>
#include <gsl/gsl_interp.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_math.h>

void interpolateFpCurrent(uint8_t **fpDataBuffers, long nFpRecs, uint8_t **hmDataBuffers, long nHmRecs, double interpolates[])
{
    long fpTimeIndex = 0, hmTimeIndex = 0;
    size_t nearestPriorFpTimeIndex;
    double dtBefore, dtAfter;
    double valueBefore, valueAfter, value;

    // Interpolating the faceplate currents to the HM times
    // Currents will be NaN when FP is not available within 0.5 s of requested HM time. 
    gsl_interp_accel *acc = gsl_interp_accel_alloc();
    // Linear interpolation
    gsl_spline *spline = gsl_spline_alloc(gsl_interp_linear, nHmRecs);
    // 
    gsl_spline_init(spline, (double*)fpDataBuffers[0], (double*)fpDataBuffers[1], nFpRecs);
    for(long hmTimeIndex = 0; hmTimeIndex < nHmRecs; hmTimeIndex++)
    {
        // Interpolate only if we have FP values within 0.5 s of each side of the requested time.
        nearestPriorFpTimeIndex = gsl_interp_bsearch((double*)fpDataBuffers[0], HMTIME(), 0, nFpRecs-1);
//        nearestPriorFpTimeIndex = gsl_interp_accel_find(acc, (double*)fpDataBuffers[0], nFpRecs, HMTIME());
        fpTimeIndex = nearestPriorFpTimeIndex;
        dtBefore = (HMTIME() - FPTIME())/1000.;
        valueBefore = ((double*)fpDataBuffers[1])[fpTimeIndex];
        fpTimeIndex++;
        if(fpTimeIndex >= nFpRecs)
            fpTimeIndex = nFpRecs-1;
        dtAfter = (FPTIME() - HMTIME())/1000.;
        valueAfter = ((double*)fpDataBuffers[1])[fpTimeIndex];
        if (dtBefore >= 0.0 && dtBefore < 0.5 && dtAfter >= 0.0 && dtAfter < 0.5)
        {
            value = valueBefore + (valueAfter - valueBefore) * dtBefore / (dtBefore + dtAfter);
        }
        else if(dtBefore >= -0.5 && dtBefore < 0.5)
        {
            // Extrapolate with constant interpolation
            // This will happen for the first time of each day even when we have full measurements
            // Better to average the 16 Hz FP currents at -7 +8 samples around the requested time
            // (Do that in a later revision)
            value = ((double*)fpDataBuffers[1])[nearestPriorFpTimeIndex];
        }
        else if(dtAfter >= -0.5 && dtAfter < 0.5)
        {
            // Extrapolate with constant interpolation
            // This will happen for the first time of each day even when we have full measurements
            // Better to average the 16 Hz FP currents at -7 +8 samples around the requested time
            // (Do that in a later revision)
            value = ((double*)fpDataBuffers[1])[fpTimeIndex];
        }
        else
        {
            value = GSL_NAN;
        }
        interpolates[hmTimeIndex] = value;
    }

    gsl_spline_free(spline);
    gsl_interp_accel_free(acc);

    return;

}

void interpolateVNEC(uint8_t **vnecDataBuffers, long nVnecRecs, uint8_t **hmDataBuffers, long nHmRecs, double interpolates[], int vnecIndex)
{
    long vnecTimeIndex = 0, hmTimeIndex = 0;
    size_t nearestPriorVnecTimeIndex;
    double dtBefore, dtAfter;
    double valueBefore, valueAfter, value;

    // Linear interpolation
    gsl_interp_accel *acc = gsl_interp_accel_alloc();
    gsl_spline *spline = gsl_spline_alloc(gsl_interp_linear, nHmRecs);

    gsl_spline_init(spline, (double*)vnecDataBuffers[0], (double*)vnecDataBuffers[vnecIndex], nVnecRecs);
    for(long hmTimeIndex = 0; hmTimeIndex < nHmRecs; hmTimeIndex++)
    {
        // Interpolate only if we have VNEC values within 1.5 s of each side of the requested time.
        nearestPriorVnecTimeIndex = gsl_interp_bsearch((double*)vnecDataBuffers[0], HMTIME(), 0, nVnecRecs-1);
        // if (hmTimeIndex < 4 && vnecIndex == 1)
        //     fprintf(stdout, "%lu\n", nearestPriorVnecTimeIndex);
        vnecTimeIndex = nearestPriorVnecTimeIndex;
        dtBefore = (HMTIME() - VNECTIME())/1000.;
        valueBefore = ((double*)vnecDataBuffers[vnecIndex])[vnecTimeIndex];
        vnecTimeIndex++;
        if(vnecTimeIndex >= nVnecRecs)
            vnecTimeIndex = nVnecRecs-1;
        dtAfter = (VNECTIME() - HMTIME())/1000.;
        valueAfter = ((double*)vnecDataBuffers[vnecIndex])[vnecTimeIndex];
        // Linear interpolation. GSL_interp always returns nan. Why?
        if (fabs(dtBefore) < 1.5 && fabs(dtAfter) < 1.5)
        {
            value = valueBefore + (valueAfter - valueBefore) * dtBefore / (dtBefore + dtAfter);
        }
        else if(fabs(dtBefore) < 2.0)
        {
            // Extrapolate with constant interpolation
            // This will happen for the first time of each day even when we have full measurements
            value = (double) ((double*)vnecDataBuffers[vnecIndex])[nearestPriorVnecTimeIndex];
        }
        else if(fabs(dtAfter) < 2.0)
        {
            // Extrapolate with constant interpolation
            // This will happen for the first time of each day even when we have full measurements
            // Better to average the 16 Hz FP currents at -7 +8 samples around the requested time
            // (Do that in a later revision)
            value = (double) ((double*)vnecDataBuffers[vnecIndex])[vnecTimeIndex];
        }
        else
        {
            value = MISSING_VNEC_VALUE;
        }
        interpolates[hmTimeIndex] = value;
    }

    gsl_spline_free(spline);
    gsl_interp_accel_free(acc);

    return;

}

void interpolateDipLatitude(double *timeIn, double * dipLatIn, long nDipLatRecs, uint8_t **hmDataBuffers, long nHmRecs, double interpolates[])
{
    long dipLatTimeIndex = 0, hmTimeIndex = 0;
    size_t nearestPriorDipLatTimeIndex;
    double dtBefore, dtAfter;
    double valueBefore, valueAfter, value;

    // Currents will be NaN when FP is not available within 0.5 s of requested HM time. 
    // Linear interpolation
    gsl_interp_accel *acc = gsl_interp_accel_alloc();
    gsl_spline *spline = gsl_spline_alloc(gsl_interp_linear, nHmRecs);

    gsl_spline_init(spline, timeIn, dipLatIn, nDipLatRecs);
    for(long hmTimeIndex = 0; hmTimeIndex < nHmRecs; hmTimeIndex++)
    {
        // Interpolate only if we have VNEC values within 1.5 s of each side of the requested time.
        nearestPriorDipLatTimeIndex = gsl_interp_bsearch(timeIn, HMTIME(), 0, nDipLatRecs-1);
        dipLatTimeIndex = nearestPriorDipLatTimeIndex;
        dtBefore = (HMTIME() - timeIn[dipLatTimeIndex])/1000.;
        valueBefore = dipLatIn[dipLatTimeIndex];
        dipLatTimeIndex++;
        if(dipLatTimeIndex >= nDipLatRecs)
            dipLatTimeIndex = nDipLatRecs-1;
        dtAfter = (timeIn[dipLatTimeIndex] - HMTIME())/1000.;
        valueAfter = dipLatIn[dipLatTimeIndex];
        // Linear interpolation.
        if (valueBefore == MISSING_DIPLAT_VALUE || valueAfter == MISSING_DIPLAT_VALUE)
        {
            // Nearby data were flagged as missing in the MAG file. Do not interpolate
            value = MISSING_DIPLAT_VALUE;
        }
        else if (fabs(dtBefore) < 1.5 && fabs(dtAfter) < 1.5)
        {
            value = valueBefore + (valueAfter - valueBefore) * dtBefore / (dtBefore + dtAfter);
        }
        else if(fabs(dtBefore) < 2.0)
        {
            // Extrapolate with constant interpolation
            // This will happen for the first time of each day even when we have full measurements
            value = dipLatIn[nearestPriorDipLatTimeIndex];
        }
        else if(fabs(dtAfter) < 2.0)
        {
            // Extrapolate with constant interpolation
            // This will happen for the first time of each day even when we have full measurements
            // Better to average the 16 Hz FP currents at -7 +8 samples around the requested time
            // (Do that in a later revision)
            value = dipLatIn[dipLatTimeIndex];
        }
        else
        {
            value = MISSING_DIPLAT_VALUE;
        }
        interpolates[hmTimeIndex] = value;
    }

    gsl_spline_free(spline);
    gsl_interp_accel_free(acc);

    return;

}
