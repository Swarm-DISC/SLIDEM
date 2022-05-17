/*

    SLIDEM Processor: slidem_settings.h

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

#ifndef _SLIDEM_SETTING_H
#define _SLIDEM_SETTING_H

#define SOFTWARE_VERSION_STRING "SLIDEM version 2022-05-17"
#define SOFTWARE_VERSION "02.02"
#define SLIDEM_PRODUCT_CODE "IDM"
#define SLIDEM_PRODUCT_TYPE "OPER"
#define SLIDEM_FILE_TYPE "EFIAIDM_2_"
#define EXPORT_VERSION_STRING "0101"

#define SLIDEM_BASE_FILENAME_LENGTH 55
#define FP_FILENAME_LENGTH 59
#define HM_FILENAME_LENGTH 59
#define MOD_FILENAME_LENGTH 59
#define MAG_FILENAME_LENGTH 70

#define FACEPLATE_VOLTAGE -3.5 // V

#define MIEFF_FROM_TBT2015_MODEL true // get estimated ion effective mass from TBT 2015 model? false implies 16 amu.
#define MODIFIED_OML_GEOMETRIES true // use Lira-Resendiz et al. modified faceplate area and Langmuir probe radius
#define MODIFIED_OML_FACEPLATE_CORRECTION true
#define MODIFIED_OML_SPHERICAL_PROBE_CORRECTION true

#define POST_PROCESS_ION_DRIFT true // remove high-latitude linear drift vs time model from ion drift
#define SLIDEM_QDLAT_CUTOFF 50 // Quasi-dipolar magnetic latitude boundary for estimating effective mass and ion drift. Calculate ion drift and ion drift complementary ion density if at or poleward of this QD latitude.
#define SLIDEM_POST_PROCESSING_QDLAT_WIDTH 1.0 // from SLIDEM_QDLAT_CUTOFF to SLIDEM_QDLAT_CUTOFF + SLIDEM_POST_PROCESSING_QDLAT_WIDTH

#define BLENDED_TE true // Use uncorrected blended electron temperature for OML calcs
#define BLENDED_VS true // Use blended satellite potential estimate for OML calcs

#define SECONDS_OF_DATA_REQUIRED_FOR_PROCESSING 1 // 1 second
#define SECONDS_OF_DATA_REQUIRED_FOR_EXPORTING 1 // 1 second
#define SECONDS_OF_BOUNDARY_DATA_REQUIRED_FOR_PROCESSING 0 // 25 minutes to ensure coverage of each fit region
#define MINIMUM_POINTS_PER_FIT_REGION 10 // at least 10 data points needed for each end of the polar pass for ion drift offset estimation

// Do not include points in the ion drift post-calibration offset model if any of the bits in the mask have been raised
//#define ION_DRIFT_POST_CALIBRATION_FLAG_MASK 63991 
// Testing offset removal using valid points only in model: As a start, we allow all points in the right lattitude range to be part of the model
// Future evolution of the processor can include refinement of the offset model
#define ION_DRIFT_POST_CALIBRATION_FLAG_MASK 0 // Allow all points regardless of validity flag

#define SLIDEM_MAX_ITERATIONS 100
#define SLIDEM_NI_ITERATION_THRESHOLD 0.01 // fraction 0 to 1
#define SLIDEM_MIEFF_ITERATION_THRESHOLD 0.01 // fraction 0 to 1
#define SLIDEM_VI_ITERATION_THRESHOLD 1.0 // m/s

#define SLIDEM_K 1.38e-23
#define SLIDEM_EPS 8.85e-12
#define SLIDEM_QE 1.602e-19
#define SLIDEM_RP 0.004 // m
#define SLIDEM_WFP 0.351 // m
#define SLIDEM_HFP 0.229 // m
#define SLIDEM_MAMU 1.66e-27 // kg 

// FLAG conditions
#define FLAGS_MAXIMUM_DRIFT_MAGNITUDE 6000.0 // 8 km/s maximum drift for flagging
#define FLAGS_MINIMUM_MIEFF 1.0 // amu
#define FLAGS_MAXIMUM_MIEFF 40.0 // amu
#define FLAGS_MINIMUM_NI 1.0e8 // m^-3
#define FLAGS_MAXIMUM_NI 2.0e13 // m^-3
#define FLAGS_MINIMUM_FACEPLATE_AREA 0.08 // m^2
#define FLAGS_MAXIMUM_FACEPLATE_AREA 0.15 // m^2
#define FLAGS_MINIMUM_PROBE_RADIUS 0.001 // m
#define FLAGS_MAXIMUM_PROBE_RADIUS 0.005 // m
#define FLAGS_MAXIMUM_PROBE_POTENTIAL_DIFFERENCE 0.3 // V (absolute value of difference in Vs_lgn and Vs_hgn)
#define FLAGS_MINIMUM_LP_SPACECRAFT_POTENTIAL -5.0 // V per EXTD LP release notes
#define FLAGS_MAXIMUM_LP_SPACECRAFT_POTENTIAL 5.0 // V
#define FLAGS_MINIMUM_LP_TE 0.0 // K
#define FLAGS_MAXIMUM_LP_TE 20000.0 // Per EXTD LP release notes
#define FLAGS_MINIMUM_LP_NI 0.0 // cm^-3
#define FLAGS_MAXIMUM_LP_NI 1.0e7 // cm^-3 per EXTD LP release notes

#define MINIMUM_VELOCITY_EPOCHS 10 // At least this many epochs needed in the MODx file
#define MAX_ALLOWED_CDF_GAP_SECONDS 86400.0 // CDF export split into separate files at gaps exceeding 10 minutes
#define NUM_FP_VARIABLES 2 // Taking only Timestamp and Current from FP file.
#define NUM_FPFILE_VARIABLES 6 // Ensure we are reading correct file format. Expecting 6 vars in FP file version 0201.
#define NUM_HM_VARIABLES 16
#define NUM_HMFILE_VARIABLES 22
#define NUM_MAG_VARIABLES 4
#define NUM_MAGFILE_VARIABLES 22
#define NUM_VNEC_VARIABLES 4
#define NUM_EXPORT_VARIABLES 23

#define MISSING_MIEFF_VALUE -1.0
#define MISSING_VI_VALUE -100000.0
#define MISSING_VNEC_VALUE -100000.0
#define MISSING_DIPLAT_VALUE -1000.0
#define MISSING_NI_VALUE -1.0
#define MISSING_RPROBE_VALUE -1.0
#define MISSING_FPAREA_VALUE -1.0
#define MISSING_ERROR_ESTIMATE_VALUE -1.0
#define MISSING_TE_VALUE -1.0
#define MISSING_VS_VALUE -1.0

#define GSL_FIT_MAXIMUM_ITERATIONS 500

#define CDF_GZIP_COMPRESSION_LEVEL 6L
#define CDF_BLOCKING_FACTOR 43200L

#endif // _SLIDEM_SETTING_H
