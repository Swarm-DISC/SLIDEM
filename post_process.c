/*

    SLIDEM Processor: post_process.c

    Copyright (C) 2024 Johnathan K Burchill

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

// Adapted from the Swarm Thermal Ion Imager Cross-track Ion Drift processor source code

#include "post_process.h"

#include "main.h"
#include "slidem_settings.h"
#include "slidem_flags.h"
#include "calculate_products.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_multifit.h>
#include <gsl/gsl_statistics_double.h>

extern char infoHeader[50];

void postProcessIonDrift(const char *slidemFilename, const char satellite, uint8_t **hmDataBuffers, double *vn, double *ve, double *vc, double *dipLatitude, double *fpCurrent, double *faceplateVoltage, double *fpAreaOML, double *rProbeOML, double *electronTemperature, double *spacecraftPotential, uint32_t *electronTemperatureSource, uint32_t *spacecraftPotentialSource, double *ionEffectiveMassTTS, double *ionDrift, double *ionDriftError, double *ionEffectiveMass, double *ionEffectiveMassError, double *ionDensity, double *ionDensityError, uint32_t *viFlags, uint32_t *mieffFlags, uint32_t *niFlags, uint16_t *iterationCount, probeParams sphericalProbeParams, long nHmRecs)
{
    fprintf(stdout, "%sPost-processing ion drift\n", infoHeader);

    // Turn off GSL failsafe error handler. We typically check the GSL return codes.
    gsl_set_error_handler_off();

    // Offset model parameters
    double lat1 = SLIDEM_QDLAT_CUTOFF;
    double lat2 = lat1 + SLIDEM_POST_PROCESSING_QDLAT_WIDTH;
    offset_model_fit_arguments fitargs[4] = {
        {0, "Northern ascending", lat1, lat2, lat2, lat1},
        {1, "Southern descending", -lat1, -lat2, -lat2, -lat1}
    };

    // Open the fit log file for writing
    char fitLogFileName[FILENAME_MAX];
    sprintf(fitLogFileName, "%s.fitlog", slidemFilename);
    FILE *fitFile = NULL;
    fitFile = fopen(fitLogFileName, "w");
    if (fitFile == NULL)
    {
        fprintf(stdout, "%sCould not open fit log file:\n  %s\nAborting post processing.\n", infoHeader, fitLogFileName);
        return;
    }
    fprintf(fitFile, "EFI IDM Along-track ion drift fit results by fit region.\n");
    fprintf(fitFile, "Each region consists of two mid-latitude segments denoted by CDF_EPOCH times T11, T12, T21, and T22.\n");
    fprintf(fitFile, "Linear models based on robust least squares (GNU Scientific Library) are subtracted from each region for which a fit can be obtained.\n");
    fprintf(fitFile, "Regions:\n");
    for (uint8_t ind = 0; ind < 2; ind++)
    {
        fprintf(fitFile, "%d %21s: (% 5.1f, % 5.1f) -> (% 5.1f, % 5.1f)\n", fitargs[ind].regionNumber, fitargs[ind].regionName, fitargs[ind].lat1, fitargs[ind].lat2, fitargs[ind].lat3, fitargs[ind].lat4);
    }
    fprintf(fitFile, "\n");
    fprintf(fitFile, "The columns are:\n");
    fprintf(fitFile, "regionNumber fitNumber numPoints1 numPoints2 T11 T12 T21 T22 offsetHX slopeHX adjRsqHX rmseHX medianHX1 medianHX2 madHX madHX1 madHX2 offsetHY slopeHY adjRsqHY rmseHY medianHY1 medianHY2 madHY madHY1 madHY2 offsetVX slopeVX adjRsqVX rmseVX medianVX1 medianVX2 madVX madVX1 madVX2 offsetVY slopeVY adjRsqVY rmseVY medianVY1 medianVY2 madVY madVY1 madVY2\n");
    fprintf(fitFile, "\n");
    fflush(fitFile);
    fflush(stdout);

    for (uint8_t ind = 0; ind < 2; ind++)
    {
        removeOffsetsAndSetFlags(satellite, fitargs[ind], nHmRecs, hmDataBuffers, vn, ve, vc, dipLatitude, fpCurrent, faceplateVoltage, fpAreaOML, rProbeOML, electronTemperature, spacecraftPotential, electronTemperatureSource, spacecraftPotentialSource, ionEffectiveMassTTS, ionDrift, ionDriftError, ionEffectiveMass, ionEffectiveMassError, ionDensity, ionDensityError, viFlags, mieffFlags, niFlags, iterationCount, sphericalProbeParams, fitFile);
    }

    fclose(fitFile);

}

void removeOffsetsAndSetFlags(const char satellite, offset_model_fit_arguments fitargs, long nHmRecs, uint8_t **hmDataBuffers, double *vn, double *ve, double *vc, double *dipLatitude, double *fpCurrent, double *faceplateVoltage, double *fpAreaOML, double *rProbeOML, double *electronTemperature, double *spacecraftPotential, uint32_t *electronTemperatureSource, uint32_t *spacecraftPotentialSource, double *ionEffectiveMassTTS, double *ionDrift, double *ionDriftError, double *ionEffectiveMass, double *ionEffectiveMassError, double *ionDensity, double *ionDensityError, uint32_t *viFlags, uint32_t *mieffFlags, uint32_t *niFlags, uint16_t *iterationCount, probeParams sphericalProbeParams, FILE* fitFile)
{
    long hmTimeIndex = 0;
    double epoch0 = HMTIME();
    double previousQDLat = QDLAT();
    double driftValue;
    bool regionBegin = false;
    long beginIndex0 = 0, beginIndex1 = 0, endIndex0 = 0, endIndex1 = 0, modelDataIndex = 0, modelDataMidPoint = 0;

    double c0, c1, cov00, cov01, cov11, sumsq;
    double driftOffset;
    double seconds, fitTime;
    int gslStatus;
    long numModelPoints, numModel1Points, numModel2Points; // data for fit, and for median calculations at each end
    long actualNumModel1Points, actualNumModel2Points, actualNumModelPoints;
    bool gotFirstModelData = false;
    bool gotStartOfSecondModelData = false;
    bool gotSecondModelData = false;
    uint16_t numFits = 0;

    char startString[EPOCH_STRING_LEN+1], stopString[EPOCH_STRING_LEN+1];

    double dlatfirst = fitargs.lat2 - fitargs.lat1;
    double dlatsecond = fitargs.lat4 - fitargs.lat3;
    int8_t firstDirection = 0;
    int8_t secondDirection = 0;
    if (dlatfirst > 0)
        firstDirection = 1;
    else if (dlatfirst < 0)
        firstDirection = -1;
    if (dlatsecond > 0)
        secondDirection = 1;
    else if (dlatsecond < 0)
        secondDirection = -1;
    double location = (fitargs.lat2 + fitargs.lat3) / 2.;
    int8_t region; // For flagging
    if (location >= 44. )
        region = 1; // North
    else if (location <= -44.)
        region = -1; // South
    else
        region = 0; // Equator

    // Buffers for mid-latitude linear fit data
    const gsl_multifit_robust_type * fitType = gsl_multifit_robust_bisquare;
    gsl_matrix *modelTimesMatrix, *cov;
    gsl_vector *modelValues, *model1Values, *model2Values, *fitCoefficients, *work1, *work2;
    gsl_multifit_robust_workspace * gslFitWorkspace;
    gsl_multifit_robust_stats stats;
    const size_t p = 2; // linear fit

    double tregion11, tregion12, tregion21, tregion22;

    bool missingFpData = false;

    double ifp = 0;
    double ni = 0;
    double vions = 0;
    double vionsram = 0;
    double mieffmodel = 0;
    double di = 0;
    double mieff = 0;
    uint32_t viFlag = 0;
    uint32_t mieffFlag = 0;
    uint32_t niFlag = 0;
    double fpArea = 0;
    double rProbe = 0;
    double te = 0;
    double vs = 0;
    int iterations = 0;

    for (hmTimeIndex = 0; hmTimeIndex < nHmRecs; hmTimeIndex++)
    {
        missingFpData |= !isfinite(fpCurrent[hmTimeIndex]);
        c0 = 0.0;
        c1 = 0.0;
        seconds = (HMTIME() - epoch0) / 1000.;
        if ((firstDirection == 1 && QDLAT() >= fitargs.lat1 && previousQDLat < fitargs.lat1) || (firstDirection == -1 && QDLAT() <= fitargs.lat1 && previousQDLat > fitargs.lat1))
        {
            // Start a new region search
            regionBegin = true; // Found start of region to remove offset from
            gotFirstModelData = false;
            gotStartOfSecondModelData = false;
            gotSecondModelData = false;
            beginIndex0 = hmTimeIndex;
            tregion11 = HMTIME();

        }
        else if (regionBegin && ((firstDirection == 1 && QDLAT() >= fitargs.lat2 && previousQDLat < fitargs.lat2) || (firstDirection == -1 && QDLAT() <= fitargs.lat2 && previousQDLat > fitargs.lat2)))
        {
            if ((HMTIME() - tregion11)/1000. < (5400. / 2.)) // Should be within 1/2 an orbit of start of segment
            {
                gotFirstModelData = true;
                beginIndex1 = hmTimeIndex;
                tregion12 = HMTIME();
            }
            else
            {
                // reset search
                gotFirstModelData = false;
                gotStartOfSecondModelData = false;
                gotSecondModelData = false;
                regionBegin = false;
            }
        }
        else if (gotFirstModelData && ((secondDirection == -1 && QDLAT() <= fitargs.lat3 && previousQDLat > fitargs.lat3) || (secondDirection == 1 && QDLAT() >= fitargs.lat3 && previousQDLat < fitargs.lat3)))
        {
            if ((HMTIME() - tregion12)/1000. < (5400. / 2.)) // Should be within 1/2 an orbit of start of segment
            {
                gotStartOfSecondModelData = true;
                endIndex0 = hmTimeIndex;
                tregion21 = HMTIME();
            }
            else
            {
                // reset search
                gotFirstModelData = false;
                gotStartOfSecondModelData = false;
                gotSecondModelData = false;
                regionBegin = false;
            }
        }
        else if (gotStartOfSecondModelData && ((secondDirection == -1 && QDLAT() <= fitargs.lat4 && previousQDLat > fitargs.lat4) || (secondDirection == 1 && QDLAT() >= fitargs.lat4 && previousQDLat < fitargs.lat4)))
        {
            if ((HMTIME() - tregion21)/1000. < (5400. / 2.)) // Should be within 1/2 an orbit of start of segment
            {
                // We have a complete region - remove linear offset model
                endIndex1 = hmTimeIndex;
                tregion22 = HMTIME();
                gotSecondModelData = true;
            }
            else
            {
                // reset search
                missingFpData = false;
                gotFirstModelData = false;
                gotStartOfSecondModelData = false;
                gotSecondModelData = false;
                regionBegin = false;
            }
            if (gotFirstModelData && gotSecondModelData && !missingFpData)
            {
                numFits++;
                // Estimating maximum number of model points, will be fewer if drifts are flagged invalid
                numModel1Points = beginIndex1 - beginIndex0;
                numModel2Points = endIndex1 - endIndex0;
                numModelPoints = numModel1Points + numModel2Points;
                fprintf(fitFile, "%d %d %ld %ld %f %f %f %f", fitargs.regionNumber, numFits, numModel1Points, numModel2Points, tregion11, tregion12, tregion21, tregion22);
                // Allocate fit buffers
                modelTimesMatrix = gsl_matrix_alloc(numModelPoints, p);
                model1Values = gsl_vector_alloc(numModel1Points);
                model2Values = gsl_vector_alloc(numModel2Points);
                work1 = gsl_vector_alloc(numModel1Points);
                work2 = gsl_vector_alloc(numModel2Points);
                modelValues = gsl_vector_alloc(numModelPoints);
                fitCoefficients = gsl_vector_alloc(p);
                cov = gsl_matrix_alloc(p, p);

                // Load times into the model data buffer
                modelDataIndex = 0;
                actualNumModel1Points = 0;
                actualNumModel2Points = 0;
                for (hmTimeIndex = beginIndex0; hmTimeIndex < beginIndex1; hmTimeIndex++)
                {
                    // Do not include drift point in model if it is flagged
                    if ((viFlags[hmTimeIndex] & ION_DRIFT_POST_CALIBRATION_FLAG_MASK) == 0)
                    {
                        actualNumModel1Points++;
                        fitTime = (HMTIME() - epoch0)/1000.;
                        gsl_matrix_set(modelTimesMatrix, modelDataIndex, 0, 1.0);
                        gsl_matrix_set(modelTimesMatrix, modelDataIndex++, 1, fitTime); // seconds from start of file
                    }
                }
                for (hmTimeIndex = endIndex0; hmTimeIndex < endIndex1; hmTimeIndex++)
                {
                    if ((viFlags[hmTimeIndex] & ION_DRIFT_POST_CALIBRATION_FLAG_MASK) == 0)
                    {
                        actualNumModel2Points++;
                        fitTime = (HMTIME() - epoch0)/1000.;
                        gsl_matrix_set(modelTimesMatrix, modelDataIndex, 0, 1.0);
                        gsl_matrix_set(modelTimesMatrix, modelDataIndex++, 1, fitTime); // seconds from start of file
                    }
                }
                // Load values into model data buffer
                modelDataIndex = 0;
                for (hmTimeIndex = beginIndex0; hmTimeIndex < beginIndex1; hmTimeIndex++)
                {
                    if ((viFlags[hmTimeIndex] & ION_DRIFT_POST_CALIBRATION_FLAG_MASK) == 0)
                    {
                        gsl_vector_set(model1Values, modelDataIndex, ionDrift[hmTimeIndex]); 
                        gsl_vector_set(modelValues, modelDataIndex++, ionDrift[hmTimeIndex]); 
                    }
                }
                modelDataMidPoint = modelDataIndex;
                for (hmTimeIndex = endIndex0; hmTimeIndex < endIndex1; hmTimeIndex++)
                {
                    if ((viFlags[hmTimeIndex] & ION_DRIFT_POST_CALIBRATION_FLAG_MASK) == 0)
                    {
                        gsl_vector_set(model2Values, modelDataIndex - modelDataMidPoint, ionDrift[hmTimeIndex]); 
                        gsl_vector_set(modelValues, modelDataIndex++, ionDrift[hmTimeIndex]); 
                    }
                }
                // Robust linear model fit and removal
                actualNumModelPoints = actualNumModel1Points + actualNumModel2Points;
                if ((actualNumModel1Points >= MINIMUM_POINTS_PER_FIT_REGION) && (actualNumModel2Points >= MINIMUM_POINTS_PER_FIT_REGION))
                {
                    gslFitWorkspace = gsl_multifit_robust_alloc(fitType, actualNumModelPoints, p);
                    gsl_multifit_robust_maxiter(GSL_FIT_MAXIMUM_ITERATIONS, gslFitWorkspace);
                    gslStatus = gsl_multifit_robust(modelTimesMatrix, modelValues, fitCoefficients, cov, gslFitWorkspace);
                    if (gslStatus)
                    {
                        toEncodeEPOCH(tregion11, 0, startString);
                        toEncodeEPOCH(tregion22, 0, stopString);
                        if (!missingFpData)
                        {
                            fprintf(stdout, "%s<GSL Fit Error: %s> for fit region from %s to %s spanning latitudes %.0f to %.0f.\n", infoHeader, gsl_strerror(gslStatus), startString, stopString, fitargs.lat1, fitargs.lat4);
                            // Print "-9999999999.GSLERRORNUMBER" for each of the nine fit parameters
                            fprintf(fitFile, " -9999999999.%d -9999999999.%d -9999999999.%d -9999999999.%d -9999999999.%d -9999999999.%d -9999999999.%d -9999999999.%d -9999999999.%d", gslStatus, gslStatus, gslStatus, gslStatus, gslStatus, gslStatus, gslStatus, gslStatus, gslStatus);
                         }
                   }
                    else
                    {
                        c0 = gsl_vector_get(fitCoefficients, 0);
                        c1 = gsl_vector_get(fitCoefficients, 1);
                        stats = gsl_multifit_robust_statistics(gslFitWorkspace);
                        gsl_multifit_robust_free(gslFitWorkspace);
                        // check median absolute deviation and median of signal
                        // Note that median calculation sorts the array, so do this last
                        double mad = stats.sigma_mad; // For full data fitted
                        double mad1 = gsl_stats_mad(model1Values->data, 1, actualNumModel1Points, work1->data); // For first segment
                        double mad2 = gsl_stats_mad(model2Values->data, 1, actualNumModel2Points, work2->data); // For last segment
                        double median1 = gsl_stats_median(model1Values->data, 1, actualNumModel1Points);
                        double median2 = gsl_stats_median(model2Values->data, 1, actualNumModel2Points);
                        fprintf(fitFile, " %f %f %f %f %f %f %f %f %f", c0, c1, stats.adj_Rsq, stats.rmse, median1, median2, mad, mad1, mad2);
                        // Remove the offsets and assign flags for this region
                        for (hmTimeIndex = beginIndex0; hmTimeIndex < endIndex1; hmTimeIndex++)
                        {
                            // remove ion drift offset
                            driftOffset = (((HMTIME() - epoch0)/1000.0) * c1 + c0);
                            if (isfinite(driftOffset) && isfinite(mad))
                            {
                                ionDrift[hmTimeIndex] -= driftOffset;
                                driftValue = ionDrift[hmTimeIndex];
                                // Assign ion drift resolution (uncertainty) estimate
                                ionDriftError[hmTimeIndex] = mad;
                                // Unset the post processing error flag bit (this was set in calculate_products.c)
                                viFlags[hmTimeIndex] &= (~SLIDEM_FLAG_POST_PROCESSING_ERROR); 

                                if (POST_PROCESS_ION_EFFECTIVE_MASS_AND_DENSITY)
                                {
                                    // Update ion effective mass and density using the estimates along-track ion drift 
                                    if(isfinite(fpCurrent[hmTimeIndex]))
                                    {
                                        vionsram = sqrt(vn[hmTimeIndex]*vn[hmTimeIndex] + ve[hmTimeIndex]*ve[hmTimeIndex] + vc[hmTimeIndex]*vc[hmTimeIndex]);
                                        vions = vionsram - ionDrift[hmTimeIndex];
                                        viFlag = viFlags[hmTimeIndex];
                                        ni = ionDensity[hmTimeIndex] * 1e6;
                                        niFlag = niFlags[hmTimeIndex];
                                        di = NI() * 1e6 / (16.0 * SLIDEM_MAMU) / vionsram * (2.0 * M_PI * SLIDEM_RP * SLIDEM_RP * SLIDEM_QE * SLIDEM_QE); // A/V
                                        ifp = -fpCurrent[hmTimeIndex] * 1e-9; // A
                                        mieff = ionEffectiveMass[hmTimeIndex];
                                        mieffFlag = mieffFlags[hmTimeIndex];
                                        fpArea = fpAreaOML[hmTimeIndex];
                                        rProbe = rProbeOML[hmTimeIndex];
                                        te = electronTemperature[hmTimeIndex];
                                        vs = spacecraftPotential[hmTimeIndex];
                                        mieffmodel = ionEffectiveMassTTS[hmTimeIndex];

                                        iterations = iterateEquations(&ni, ni, &vions, &mieff, &viFlag, &mieffFlag, &niFlag, &fpArea, &rProbe, te, vs, faceplateVoltage[hmTimeIndex], sphericalProbeParams, ifp, di, vionsram, mieffmodel, QDLAT(), true, satellite);

                                        updateFlags(iterations, &mieff, &ionEffectiveMassError[hmTimeIndex], &ionDrift[hmTimeIndex], &ionDensityError[hmTimeIndex], &ni, &ionDensityError[hmTimeIndex], &fpAreaOML[hmTimeIndex], &rProbeOML[hmTimeIndex], te, vs, electronTemperatureSource[hmTimeIndex], spacecraftPotentialSource[hmTimeIndex], vionsram, dipLatitude[hmTimeIndex], vn, ve, vc, &mieffFlag, NULL, &niFlag, NULL, hmDataBuffers, hmTimeIndex);

                                        ionDensity[hmTimeIndex] = ni / 1e6;
                                        niFlags[hmTimeIndex] = niFlag;
                                        ionEffectiveMass[hmTimeIndex] = mieff;
                                        mieffFlags[hmTimeIndex] = mieffFlag;
                                        fpAreaOML[hmTimeIndex] = fpArea;
                                        rProbeOML[hmTimeIndex] = rProbe;
                                        iterationCount[hmTimeIndex] += iterations;


                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (!missingFpData)
                    {
                        fprintf(stdout, "%s Fit error: did not get enough fit points for region defined for CDF_EPOCHS %f, %f, %f, %f: not fitting and not removing offsets.\n", infoHeader, tregion11, tregion12, tregion21, tregion22);
                    }
                }
                fprintf(fitFile, "\n");
                gsl_matrix_free(modelTimesMatrix);
                gsl_vector_free(model1Values);
                gsl_vector_free(model2Values);
                gsl_vector_free(modelValues);
                gsl_vector_free(work1);
                gsl_vector_free(work2);
                gsl_vector_free(fitCoefficients);
                gsl_matrix_free(cov);
            }
            else
            {
                if (!missingFpData)
                {
                    fprintf(stdout, "%s Fit error: did not get both endpoints of region defined for CDF_EPOCHS %f, %f, %f, %f: not fitting and not removing offsets.\n", infoHeader, tregion11, tregion12, tregion21, tregion22);
                }
            }
            
            missingFpData = false;
            regionBegin = false;
            gotFirstModelData = false;
            gotStartOfSecondModelData = false;
            gotSecondModelData = false;
            if (hmTimeIndex == nHmRecs)
            {
                break;
            }
            else
            {
                hmTimeIndex++;
            }
            
        }

        previousQDLat = QDLAT();
    
    }


}
