/*

    SLIDEM Processor: calculate_products.h

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

#ifndef _CALCULATE_PRODUCTS_H_
#define _CALCULATE_PRODUCTS_H_

#include <stdint.h>

#include "modified_oml.h"

void calculateProducts(const char satellite, uint8_t **hmDataBuffers, double *fpCurrent, double *vn, double *ve, double *vc, double *dipLatitude, double *faceplateVoltage, double f107Adj, int dayOfYear, double *ionEffectiveMass, double *ionDensity, double *ionDriftRaw, double *ionDrift, double *IonEffectiveMassError, double *ionDensityError, double *ionDriftError, double *fpAreaOML, double *rProbeOML, double *electronTemperature, double *spacecraftPotential, double *ionEffectiveMassTTS, uint32_t *mieffFlags, uint32_t *viFlags, uint32_t *niFlags, uint16_t *iterationCount, long nHmRecs, faceplateParams fpParams, probeParams sphericalProbeParams, long *numberOfSlidemEstimates);

void getTeVs(const char satellite, uint8_t **hmDataBuffers, long hmTimeIndex, double *te, uint32_t *teSource, double *vs, uint32_t *vsSource);


enum LP_FLAGS {
    LP_HGN_OVERFLOW_LINEAR_BIAS = 1 << 2,
    LP_LGN_OVERFLOW_LINEAR_BIAS = 1 << 3,
    LP_HGN_OVERFLOW_RETARDED_BIAS = 1 << 4,
    LP_LGN_OVERFLOW_RETARDED_BIAS = 1 << 5,
    LP_HGN_ZERO_TRACKING_FAILED = 1 << 6,
    LP_LGN_ZERO_TRACKING_FAILED = 1 << 7,
    LP_HGN_LINEAR_BIAS_LESS_THAN_RETARDED_BIAS = 1 << 9,
    LP_LGN_LINEAR_BIAS_LESS_THAN_RETARDED_BIAS = 1 << 10,
    LP_HGN_LINEAR_BIAS_GREATER_THAN_5V_16BIT_OVERFLOW = 1 << 11,
    LP_LGN_LINEAR_BIAS_GREATER_THAN_5V_16BIT_OVERFLOW = 1 << 12,
    LP_HGN_ION_ADMITTANCE_GREATER_THAN_RETARDED_ADMITTANCE = 1 << 13,
    LP_LGN_ION_ADMITTANCE_GREATER_THAN_RETARDED_ADMITTANCE = 1 << 14,
    LP_HGN_ION_CURRENT_GREATER_THAN_RETARDED_CURRENT = 1 << 15,
    LP_LGN_ION_CURRENT_GREATER_THAN_RETARDED_CURRENT = 1 << 16,
    LP_HGN_RETARDED_ADMITTANCE_GREATER_THAN_LINEAR_ADMITTANCE = 1 << 17,    
    LP_LGN_RETARDED_ADMITTANCE_GREATER_THAN_LINEAR_ADMITTANCE = 1 << 18,
    LP_HGN_RETARDED_CURRENT_GREATER_THAN_LINEAR_CURRENT = 1 << 19,
    LP_LGN_RETARDED_CURRENT_GREATER_THAN_LINEAR_CURRENT = 1 << 20,
    LP_NE_FROM_LGN_PROBE = 1 << 21
};

enum LP_SOURCE_PROBE {
    LP_HGN_PROBE = 0b01,
    LP_LGN_PROBE = 0b10,
    LP_NO_PROBE = 0b00,
    LP_BLENDED_PROBE = 0b11
};

enum LP_VALIDITY_MASKS {
    LP_TE_HGN_MASK = (
        LP_HGN_OVERFLOW_LINEAR_BIAS
      | LP_HGN_OVERFLOW_RETARDED_BIAS
      | LP_HGN_ZERO_TRACKING_FAILED
      | LP_HGN_ION_ADMITTANCE_GREATER_THAN_RETARDED_ADMITTANCE
      | LP_HGN_ION_CURRENT_GREATER_THAN_RETARDED_CURRENT
      ),
    LP_TE_LGN_MASK = (
        LP_LGN_OVERFLOW_LINEAR_BIAS 
      | LP_LGN_OVERFLOW_RETARDED_BIAS 
      | LP_LGN_ZERO_TRACKING_FAILED 
      | LP_LGN_ION_ADMITTANCE_GREATER_THAN_RETARDED_ADMITTANCE 
      | LP_LGN_ION_CURRENT_GREATER_THAN_RETARDED_CURRENT
      ),
    LP_VS_HGN_MASK = (
        LP_HGN_OVERFLOW_LINEAR_BIAS 
      | LP_HGN_OVERFLOW_RETARDED_BIAS 
      | LP_HGN_ZERO_TRACKING_FAILED 
      | LP_HGN_LINEAR_BIAS_LESS_THAN_RETARDED_BIAS 
      | LP_HGN_LINEAR_BIAS_GREATER_THAN_5V_16BIT_OVERFLOW 
      | LP_HGN_ION_ADMITTANCE_GREATER_THAN_RETARDED_ADMITTANCE 
      | LP_HGN_ION_CURRENT_GREATER_THAN_RETARDED_CURRENT 
      | LP_HGN_RETARDED_ADMITTANCE_GREATER_THAN_LINEAR_ADMITTANCE 
      | LP_HGN_RETARDED_CURRENT_GREATER_THAN_LINEAR_CURRENT
      ),
    LP_VS_LGN_MASK = (
        LP_LGN_OVERFLOW_LINEAR_BIAS 
      | LP_LGN_OVERFLOW_RETARDED_BIAS 
      | LP_LGN_ZERO_TRACKING_FAILED 
      | LP_LGN_LINEAR_BIAS_LESS_THAN_RETARDED_BIAS 
      | LP_LGN_LINEAR_BIAS_GREATER_THAN_5V_16BIT_OVERFLOW 
      | LP_LGN_ION_ADMITTANCE_GREATER_THAN_RETARDED_ADMITTANCE 
      | LP_LGN_ION_CURRENT_GREATER_THAN_RETARDED_CURRENT 
      | LP_LGN_RETARDED_ADMITTANCE_GREATER_THAN_LINEAR_ADMITTANCE 
      | LP_LGN_RETARDED_CURRENT_GREATER_THAN_LINEAR_CURRENT
      )
};


#endif // _CALCULATE_PRODUCTS_H_
