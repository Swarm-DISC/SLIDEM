/*

    SLIDEM Processor: load_satellite_velocity.h

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

#ifndef _LOAD_SATELLITE_VELOCITY_H
#define _LOAD_SATELLITE_VELOCITY_H

#include <stdint.h>

enum SAT_VEL_ERRORS {
    SAT_VEL_OK = 0,
    SAT_VEL_ERROR_FILE = -1,
    SAT_VEL_ERROR_UNAVAILABLE = -2,
    SAT_VEL_ERROR_TOO_FEW_EPOCHS = -3,
    SAT_VEL_ERROR_MEMORY = -4,
    SAT_VEL_ERROR_WRONG_NUMBER_OF_RECORDS_READ = -5
};

// Using long to be consistent with CDF epoch parsing in slidem.c
int loadSatelliteVelocity(const char *modFilename, uint8_t **vnecDataBuffers, long *nVnecRecs);

#endif // _LOAD_SATELLITE_VELOCITY_H
