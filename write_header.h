/*

    SLIDEM Processor: write_header.h

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


#ifndef WRITE_HEADER_H
#define WRITE_HEADER_H

#include <time.h>

#define UTC_DATE_LENGTH 24

enum HEADER_STATUS
{
    HEADER_OK = 0,
    HEADER_CREATE = -1,
    HEADER_START = -2,
    HEADER_WRITE_ERROR = -3,
    HEADER_FILENAME = -4,
    HEADER_CDFFILEINFO = -5
};


int writeSlidemHeader(const char *slidemFilename, const char *fpFilename, const char *hmFilename, const char *modFilename, const char *modFilenamePrevious, const char *magFilename, time_t processingStartTime, double firstMeasurementTime, double lastMeasurementTime, long nVnecRecsPrev);


#endif // WRITE_HEADER_H
