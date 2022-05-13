/*

    SLIDEM Processor: slidem_flags.h

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

#ifndef _SLIDEM_FLAGS_H
#define _SLIDEM_FLAGS_H

// 32 bit flags
enum SLIDEM_PRODUCT_FLAGS {
    SLIDEM_FLAG_ESTIMATE_OK = 0,
    SLIDEM_FLAG_NO_FACEPLATE_CURRENT = 1,
    SLIDEM_FLAG_ESTIMATE_DID_NOT_CONVERGE = 1<<1,
    SLIDEM_FLAG_PRODUCT_ESTIMATE_NOT_FINITE = 1<<2,
    SLIDEM_FLAG_UNCERTAINTY_ESTIMATE_NOT_FINITE = 1<<3,
    SLIDEM_FLAG_FACEPLATE_AREA_ESTIMATE_NOT_FINITE = 1<<4,
    SLIDEM_FLAG_PROBE_RADIUS_ESTIMATE_NOT_FINITE = 1<<5,
    SLIDEM_FLAG_BEYOND_VALID_QDLATITUDE = 1<<6,
    SLIDEM_FLAG_OML_FACEPLATE_AREA_CORRECTION_INVALID = 1<<7,
    SLIDEM_FLAG_OML_PROBE_RADIUS_CORRECTION_INVALID = 1<<8,
    SLIDEM_FLAG_ESTIMATE_TOO_LARGE = 1<<9,
    SLIDEM_FLAG_ESTIMATE_TOO_SMALL = 1<<10,
    SLIDEM_FLAG_LP_INPUTS_INVALID = 1<<11,
    SLIDEM_FLAG_LP_PROBE_POTENTIAL_DIFFERENCE_TOO_LARGE = 1<<12,
    SLIDEM_FLAG_SPACECRAFT_POTENTIAL_TOO_NEGATIVE = 1<<13,
    SLIDEM_FLAG_SPACECRAFT_POTENTIAL_TOO_POSITIVE = 1<<14,
    SLIDEM_FLAG_NO_SATELLITE_VELOCITY = 1<<15,
    SLIDEM_FLAG_POST_PROCESSING_ERROR = 1<<16,
    SLIDEM_FLAG_MAG_INPUT_INVALID = 1<<17
};


#endif // _SLIDEM_FLAGS_H
