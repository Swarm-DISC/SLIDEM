/*

    SLIDEM Processor: downsample.c

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

#include "downsample.h"
#include "main.h"
#include "slidem_settings.h"

#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

void downSample(uint8_t **dataBuffers, int nBuffers, long *nRecs)
{
    long timeIndex = 0, storageIndex = 0;
    double timeBuf = 0.0, t0 = 0.0, dt = 0.0;
    double *valueBuf;

    valueBuf = (double*)malloc((size_t) ((nBuffers-1) * sizeof(double)));

    int halfSecondCounter = 0;
    char timeStr[EPOCH_STRING_LEN+1];
    for (long timeIndex = 0; timeIndex < *nRecs; timeIndex++)
    {
        // Accumulate
        timeBuf += TIME();
        for (int i = 0; i < nBuffers-1; i++)
        {
            valueBuf[i] += (double)(((double*)dataBuffers[i+1])[timeIndex]);
        }
        halfSecondCounter++;

        // Average
        t0 = TIME()/1000.;
        dt = t0 - floor(t0);
        if ((dt >= 0.4375 && dt < 0.5) || (dt>= 0.9375 && dt < 1)) // Last sample of 8 in a half second
        {
            // Calculate averages from accumulated buffers and overwrite data buffers
            // time
            double tAve = timeBuf / (double) halfSecondCounter;
            double average = 0.;
            *((double*)dataBuffers[0] + storageIndex) = tAve;
            // encodeEPOCH(tAve, timeStr);
            // fprintf(stdout, "%s:", timeStr);
            for (int i = 0; i < nBuffers-1; i++)
            {
                average = valueBuf[i] / (double) halfSecondCounter;
                (((double*)dataBuffers[i+1])[storageIndex]) = average;
                // fprintf(stdout, " buf[%d] = %f", i+1, ((double*)dataBuffers[i+1])[storageIndex]);
            }
            // fprintf(stdout, "\n");
            storageIndex++;

            halfSecondCounter = 0;
            timeBuf = 0.0;
            for (int i = 0; i < nBuffers-1; i++)
            {
                valueBuf[i] = 0.0;
            }
        }
    }

    free(valueBuf);

    // Number of 2 Hz samples
    *nRecs = storageIndex;

    return;

}

