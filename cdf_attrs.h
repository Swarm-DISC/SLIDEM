/*

    SLIDEM Processor: cdf_attrs.h

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

#ifndef CDF_ATTRS_H
#define CDF_ATTRS_H

#include <cdf.h>


CDFstatus addgEntry(CDFid id, long attrNum, long entryNum, const char *entry);


typedef struct varAttr {
    char * name;
    char * type;
    char * units;
    char * desc;
    double validMin;
    double validMax;
    char * format;
} varAttr;

CDFstatus addVariableAttributes(CDFid id, varAttr attr);

void addAttributes(CDFid id, const char *softwareVersion, const char satellite, const char *version, double minTime, double maxTime, const char *slidemFilename, const char *fpFilename, const char *hmFilename, const char *modFilename, const char *modFilenamePrevious, const char *magFilename, long nVnecRecsPrev);


#endif // CDF_ATTRS_H
