/*

    SLIDEM Processor: calculate_diplatitude.c
    
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

#include "calculate_diplatitude.h"

#include "main.h"
#include "slidem_settings.h"

#include <math.h>

// From forumula in Laundal and Richmond (2017) Space Sci Rev 206:27-59

void calculateDipLatitude(uint8_t ** magDataBuffers, long nMagRecs, double * dipLat)
{
   

    double bh;
    double bz;
    for (int magTimeIndex = 0; magTimeIndex < nMagRecs; magTimeIndex++)
    {
        if (MAGFLAGSB() == 255 || MAGFLAGSQ() == 255)
        {
            // per L1b product definition, B will zeros in this case
            *dipLat = MISSING_DIPLAT_VALUE;
        }
        else
        {
            bz = BC();
            bh = sqrt(BN()*BN() + BE()*BE());
            dipLat[magTimeIndex] = atan(bz / (2.0 * bh)) * 180.0 / M_PI;
        }

    }

    return;

}
