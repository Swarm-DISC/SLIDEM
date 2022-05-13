/*

    SLIDEM Processor: calculate_products.c

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

#include "calculate_products.h"

#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "slidem_settings.h"
#include "slidem_flags.h"
#include "modified_oml.h"
#include "ioncomposition.h"

#include <gsl/gsl_math.h>

void calculateProducts(const char satellite, uint8_t **hmDataBuffers, double *fpCurrent, double *vn, double *ve, double *vc, double *dipLatitude, double *faceplateVoltage, double f107Adj, int yearDay, double *ionEffectiveMass, double *ionDensity, double *ionDriftRaw, double *ionDrift, double *ionEffectiveMassError, double *ionDensityError, double *ionDriftError, double *fpAreaOML, double *rProbeOML, double *electronTemperature, double *spacecraftPotential, double *ionEffectiveMassTBT, uint32_t *mieffFlags, uint32_t *viFlags, uint32_t *niFlags, uint16_t *iterationCount, long nHmRecs, faceplateParams fpParams, probeParams sphericalProbeParams, long *numberOfSlidemEstimates)
{
    double fpArea;
    double rProbe;
    double dipLat;
    double mieff, miefflast, mikg, mieffmodel, mimodelkg;
    double vions, vionslast; // In reference frame moving with satellite
    double vionsram;
    double alongtrackiondrift;
    double ni, nilast;
    double di;
    double ifp;
    double te;
    double vs;
    double aFpGeo = SLIDEM_WFP * SLIDEM_HFP;
    double mieffError = MISSING_ERROR_ESTIMATE_VALUE;
    double vionsError = MISSING_ERROR_ESTIMATE_VALUE;
    double niError = MISSING_ERROR_ESTIMATE_VALUE;
    uint32_t teSource = 0;
    uint32_t vsSource = 0;
    uint32_t vsFlag = 0;
    uint32_t mieffFlag = 0;
    uint32_t viFlag = 0;
    uint32_t niFlag = 0;
    long slidemEstimates = 0;

    for (long hmTimeIndex = 0; hmTimeIndex < nHmRecs; hmTimeIndex++)
    {
        int iterations = 0;
        dipLat = dipLatitude[hmTimeIndex];
        if (MIEFF_FROM_TBT2015_MODEL)
        {
            // Truhlik et al. (2015) Towards better description of solar activity variation in the
            // International Reference Ionosphere topside ion composition model, Advances in Space 
            // Research, 55, 8, 2099--2105.
            mieffmodel = ionEffectiveMassIriTBT(HEIGHT()/1000., dipLat, MLAT(), MLT(), f107Adj, yearDay);
        }
        else
        {
            mieffmodel = 16.0;
        }
        mimodelkg = mieffmodel * SLIDEM_MAMU;
        mieff = mieffmodel; // seed for effective mass as low latitude, baseline for high latitude ion drift estimate
        mieffError = 0.0;
        mieffFlag = 0;
        // Magnitude of satellite velocity
        vionsram = sqrt(vn[hmTimeIndex]*vn[hmTimeIndex] + ve[hmTimeIndex]*ve[hmTimeIndex] + vc[hmTimeIndex]*vc[hmTimeIndex]);
        vions = vionsram;
        vionsError = 0.0;
        // Set this flag bit, assuming post-processing offset corrections are not done.
        // Will be unset in post_process_ion_drift.c if an offset model could be estimated.
        viFlag = SLIDEM_FLAG_POST_PROCESSING_ERROR; 
        ni = NI() * 1e6;
        niError = 0.0;
        niFlag = 0;
        di = ni / (16.0 * SLIDEM_MAMU) / vionsram * (2.0 * M_PI * SLIDEM_RP * SLIDEM_RP * SLIDEM_QE * SLIDEM_QE); // A/V
        vionslast = -10000000.;
        miefflast = -10000000.;
        nilast = -10000000.;
        ifp = -fpCurrent[hmTimeIndex] * 1e-9; // A

        // Get Te and Vs
        getTeVs(satellite, hmDataBuffers, hmTimeIndex, &te, &teSource, &vs, &vsSource);
        electronTemperature[hmTimeIndex] = te;
        spacecraftPotential[hmTimeIndex] = vs;

        // Process sample
        if(isfinite(ifp))
        {
            while (
                iterations < SLIDEM_MAX_ITERATIONS && 
                !(
                    fabs(ni - nilast) < nilast * SLIDEM_NI_ITERATION_THRESHOLD &&
                    fabs(vions - vionslast) < SLIDEM_VI_ITERATION_THRESHOLD &&
                    fabs(mieff - miefflast) < mieff * SLIDEM_MIEFF_ITERATION_THRESHOLD
                ) // Correct version. Mathematica version would stop if only one parameter converged
            )
            {
                // Reset reference values for for triggering out of the iteration
                vionslast = vions;
                miefflast = mieff;
                mikg = mieff * SLIDEM_MAMU;
                nilast = ni;

                if (MODIFIED_OML_GEOMETRIES)
                {
                    // revise estimates of probe effective geometries
                    if (MODIFIED_OML_FACEPLATE_CORRECTION)
                        fpArea = faceplateArea(ni, te, vs, mieff, vions, faceplateVoltage[hmTimeIndex], fpParams);
                    else
                        fpArea = aFpGeo;
                    
                    if (MODIFIED_OML_SPHERICAL_PROBE_CORRECTION)
                        rProbe = probeRadius(ni, te, vs, mieff, vions, sphericalProbeParams);
                    else
                        rProbe = SLIDEM_RP;
                }
                else
                {
                    fpArea = aFpGeo;
                    rProbe = SLIDEM_RP;
                }

                // Try to estimate even if OML model is wrong (i.e., NAN from sqrt of negative numbers)
                // but leave as nan on the last iteration
                if (!isfinite(fpArea) && iterations < (SLIDEM_MAX_ITERATIONS - 1)) fpArea = aFpGeo;
                if (!isfinite(rProbe) && iterations < (SLIDEM_MAX_ITERATIONS - 1)) rProbe = SLIDEM_RP;

                // Estimate effective mass at all latitudes
                mieff = (4.0 * M_PI * rProbe * rProbe * SLIDEM_QE * ifp) / (2.0 * fpArea * di * vionsram * vionsram) / SLIDEM_MAMU;
                if (!isfinite(mieff) && iterations < (SLIDEM_MAX_ITERATIONS - 1)) mieff = mieffmodel;


                // Ion drift at high latitude
                if (fabs(QDLAT()) >= SLIDEM_QDLAT_CUTOFF)
                {
                    vions = sqrt((4.0 * M_PI * rProbe * rProbe * SLIDEM_QE * ifp) / (2.0 * fpArea * di * mimodelkg));
                    if (!isfinite(vions) && iterations < (SLIDEM_MAX_ITERATIONS - 1)) vions = vionsram;
                    mieffFlag |= SLIDEM_FLAG_BEYOND_VALID_QDLATITUDE;

                    ni = sqrt(2.0 * ifp * di * mimodelkg / (fpArea * 4.0 * M_PI * rProbe * rProbe * SLIDEM_QE * SLIDEM_QE * SLIDEM_QE));
                    if (!isfinite(ni) && iterations < (SLIDEM_MAX_ITERATIONS - 1)) ni = ifp / (aFpGeo * SLIDEM_QE * vions);
                    if (!isfinite(ni) && iterations < (SLIDEM_MAX_ITERATIONS - 1)) ni = NI() * 1.0e6;
                }
                else // effective mass estimates are intended for low latitude
                {
                    viFlag |= SLIDEM_FLAG_BEYOND_VALID_QDLATITUDE;
                    ni = ifp / (fpArea * SLIDEM_QE * vionsram);
                    if (!isfinite(ni) && iterations < (SLIDEM_MAX_ITERATIONS - 1)) ni = ifp / (aFpGeo * SLIDEM_QE * vionsram);
                    // Check again, in case aFpGeo is not finite
                    if (!isfinite(ni) && iterations < (SLIDEM_MAX_ITERATIONS - 1)) ni = NI() * 1.0e6;
                }

                // TODO Calculate error estimates and flags

                iterations++;
            }
            if (fabs(QDLAT()) > SLIDEM_QDLAT_CUTOFF)
                alongtrackiondrift = vionsram - vions; // positive in direction of satellite velocity vector
            else
                alongtrackiondrift = MISSING_VI_VALUE;
            if (iterations >= SLIDEM_MAX_ITERATIONS)
            {
                mieffFlag |= SLIDEM_FLAG_ESTIMATE_DID_NOT_CONVERGE;
                viFlag |= SLIDEM_FLAG_ESTIMATE_DID_NOT_CONVERGE;
                niFlag |= SLIDEM_FLAG_ESTIMATE_DID_NOT_CONVERGE;
            }
            else
            {
                slidemEstimates++;
            }

            // Replace nans for CDF export
            if (isfinite(mieff))
            {
                if (mieff > FLAGS_MAXIMUM_MIEFF)
                    mieffFlag |= SLIDEM_FLAG_ESTIMATE_TOO_LARGE;
                else if (mieff < FLAGS_MINIMUM_MIEFF)
                    mieffFlag |= SLIDEM_FLAG_ESTIMATE_TOO_SMALL;
            }
            else
            {
                mieff = MISSING_MIEFF_VALUE;
                mieffFlag |= SLIDEM_FLAG_PRODUCT_ESTIMATE_NOT_FINITE;
            }
            if (!isfinite(mieffError))
            {
                mieffError = MISSING_ERROR_ESTIMATE_VALUE;
                mieffFlag |= SLIDEM_FLAG_UNCERTAINTY_ESTIMATE_NOT_FINITE;
            }
            if (isfinite(alongtrackiondrift))
            {
                if (fabs(alongtrackiondrift) > FLAGS_MAXIMUM_DRIFT_MAGNITUDE)
                    viFlag |= SLIDEM_FLAG_ESTIMATE_TOO_LARGE;
            }
            else
            {
                alongtrackiondrift = MISSING_VI_VALUE;
                viFlag |= SLIDEM_FLAG_PRODUCT_ESTIMATE_NOT_FINITE;
            }
            if (!isfinite(vionsError))
            {
                vionsError = MISSING_ERROR_ESTIMATE_VALUE;
                viFlag |= SLIDEM_FLAG_UNCERTAINTY_ESTIMATE_NOT_FINITE;
            }
            if (isfinite(ni))
            {
                if (ni > FLAGS_MAXIMUM_NI)
                    niFlag |= SLIDEM_FLAG_ESTIMATE_TOO_LARGE;
                else if (niFlag < FLAGS_MINIMUM_NI)
                    niFlag |= SLIDEM_FLAG_ESTIMATE_TOO_SMALL;
            }
            else
            {
                ni = MISSING_NI_VALUE;
                niFlag |= SLIDEM_FLAG_PRODUCT_ESTIMATE_NOT_FINITE;
            }
            if (!isfinite(niError))
            {
                niError = MISSING_ERROR_ESTIMATE_VALUE;
                niFlag |= SLIDEM_FLAG_UNCERTAINTY_ESTIMATE_NOT_FINITE;
            }
            if (isfinite(fpArea))
            {
                if (fpArea > FLAGS_MAXIMUM_FACEPLATE_AREA || fpArea < FLAGS_MINIMUM_FACEPLATE_AREA)
                {
                    mieffFlag |= SLIDEM_FLAG_OML_FACEPLATE_AREA_CORRECTION_INVALID;
                    viFlag |= SLIDEM_FLAG_OML_FACEPLATE_AREA_CORRECTION_INVALID;
                    niFlag |= SLIDEM_FLAG_OML_FACEPLATE_AREA_CORRECTION_INVALID;
                }
            }
            else
            {
                fpArea = MISSING_FPAREA_VALUE;
                mieffFlag |= SLIDEM_FLAG_OML_FACEPLATE_AREA_CORRECTION_INVALID;
                mieffFlag |= SLIDEM_FLAG_FACEPLATE_AREA_ESTIMATE_NOT_FINITE;
                viFlag |= SLIDEM_FLAG_OML_FACEPLATE_AREA_CORRECTION_INVALID;
                viFlag |= SLIDEM_FLAG_FACEPLATE_AREA_ESTIMATE_NOT_FINITE;
                niFlag |= SLIDEM_FLAG_OML_FACEPLATE_AREA_CORRECTION_INVALID;
                niFlag |= SLIDEM_FLAG_FACEPLATE_AREA_ESTIMATE_NOT_FINITE;
            }
            if (isfinite(rProbe))
            {
                if (rProbe > FLAGS_MAXIMUM_PROBE_RADIUS || rProbe < FLAGS_MINIMUM_PROBE_RADIUS)
                {
                    mieffFlag |= SLIDEM_FLAG_OML_FACEPLATE_AREA_CORRECTION_INVALID;
                    viFlag |= SLIDEM_FLAG_OML_FACEPLATE_AREA_CORRECTION_INVALID;
                    niFlag |= SLIDEM_FLAG_OML_FACEPLATE_AREA_CORRECTION_INVALID;
                }
            }
            else
            {
                rProbe = MISSING_RPROBE_VALUE;
                mieffFlag |= SLIDEM_FLAG_OML_PROBE_RADIUS_CORRECTION_INVALID;
                mieffFlag |= SLIDEM_FLAG_PROBE_RADIUS_ESTIMATE_NOT_FINITE;
                viFlag |= SLIDEM_FLAG_OML_PROBE_RADIUS_CORRECTION_INVALID;
                viFlag |= SLIDEM_FLAG_PROBE_RADIUS_ESTIMATE_NOT_FINITE;
                niFlag |= SLIDEM_FLAG_OML_PROBE_RADIUS_CORRECTION_INVALID;
                niFlag |= SLIDEM_FLAG_PROBE_RADIUS_ESTIMATE_NOT_FINITE;
            }

            // LP validity checks
            // Potential difference between spherical probes too large? 
            if (fabs(VSHGN() - VSLGN()) > FLAGS_MAXIMUM_PROBE_POTENTIAL_DIFFERENCE)
            {
                mieffFlag |= SLIDEM_FLAG_LP_PROBE_POTENTIAL_DIFFERENCE_TOO_LARGE;
                viFlag |= SLIDEM_FLAG_LP_PROBE_POTENTIAL_DIFFERENCE_TOO_LARGE;
                niFlag |= SLIDEM_FLAG_LP_PROBE_POTENTIAL_DIFFERENCE_TOO_LARGE;
            }
            // Spacecraft potential too negative?
            if (vs < FLAGS_MINIMUM_LP_SPACECRAFT_POTENTIAL)
            {
                mieffFlag |= SLIDEM_FLAG_SPACECRAFT_POTENTIAL_TOO_NEGATIVE;
                mieffFlag |= SLIDEM_FLAG_LP_INPUTS_INVALID;
                viFlag |= SLIDEM_FLAG_SPACECRAFT_POTENTIAL_TOO_NEGATIVE;
                viFlag |= SLIDEM_FLAG_LP_INPUTS_INVALID;
                niFlag |= SLIDEM_FLAG_SPACECRAFT_POTENTIAL_TOO_NEGATIVE;
                niFlag |= SLIDEM_FLAG_LP_INPUTS_INVALID;
            }
            else if (vs > FLAGS_MAXIMUM_LP_SPACECRAFT_POTENTIAL)
            {
                mieffFlag |= SLIDEM_FLAG_SPACECRAFT_POTENTIAL_TOO_POSITIVE;
                mieffFlag |= SLIDEM_FLAG_LP_INPUTS_INVALID;
                viFlag |= SLIDEM_FLAG_SPACECRAFT_POTENTIAL_TOO_POSITIVE;
                viFlag |= SLIDEM_FLAG_LP_INPUTS_INVALID;
                niFlag |= SLIDEM_FLAG_SPACECRAFT_POTENTIAL_TOO_POSITIVE;
                niFlag |= SLIDEM_FLAG_LP_INPUTS_INVALID;
            }
            // LP Probe issues?
            if (te < FLAGS_MINIMUM_LP_TE || te > FLAGS_MAXIMUM_LP_TE || ni < FLAGS_MINIMUM_NI || ni > FLAGS_MAXIMUM_NI || teSource == LP_NO_PROBE || vsSource == LP_NO_PROBE)
            {
                mieffFlag |= SLIDEM_FLAG_LP_INPUTS_INVALID;
                viFlag |= SLIDEM_FLAG_LP_INPUTS_INVALID;
                niFlag |= SLIDEM_FLAG_LP_INPUTS_INVALID;
            }
            // Satellite velocity data available?
            if (!isfinite(vionsram))
            {
                // overwrite VNEC and raise flags
                vn[hmTimeIndex] = MISSING_VNEC_VALUE;
                ve[hmTimeIndex] = MISSING_VNEC_VALUE;
                vc[hmTimeIndex] = MISSING_VNEC_VALUE;
                mieffFlag |= SLIDEM_FLAG_NO_SATELLITE_VELOCITY;
                viFlag |= SLIDEM_FLAG_NO_SATELLITE_VELOCITY;
                niFlag |= SLIDEM_FLAG_NO_SATELLITE_VELOCITY;
            }
            // DIP Latitude missing? Has to be due to a problem with input MAG data
            if (dipLat == MISSING_DIPLAT_VALUE)
            {
                mieffFlag |= SLIDEM_FLAG_MAG_INPUT_INVALID;
                viFlag |= SLIDEM_FLAG_MAG_INPUT_INVALID;
                niFlag |= SLIDEM_FLAG_MAG_INPUT_INVALID;
            }

        }
        else
        {
            // ifp is NAN means there were no IFP measurements close enough to interpolate for this time
            mieff = MISSING_MIEFF_VALUE;
            mieffError = MISSING_ERROR_ESTIMATE_VALUE;
            mieffFlag |= SLIDEM_FLAG_NO_FACEPLATE_CURRENT;
            mieffmodel = MISSING_MIEFF_VALUE;
            alongtrackiondrift = MISSING_VI_VALUE;
            vionsError = MISSING_ERROR_ESTIMATE_VALUE;
            viFlag |= SLIDEM_FLAG_NO_FACEPLATE_CURRENT;
            ni = MISSING_NI_VALUE;
            niError = MISSING_ERROR_ESTIMATE_VALUE;
            niFlag |= SLIDEM_FLAG_NO_FACEPLATE_CURRENT;
            fpArea = MISSING_FPAREA_VALUE;
            rProbe = MISSING_RPROBE_VALUE;
            iterations = 0;
        }

        // Return estimate for all latitudes, though flagged invalid at high latitude
        ionEffectiveMass[hmTimeIndex] = mieff; // a.m.u.
        ionEffectiveMassError[hmTimeIndex] = mieffError;
        ionEffectiveMassTBT[hmTimeIndex] = mieffmodel;
        mieffFlags[hmTimeIndex] = mieffFlag;

        ionDrift[hmTimeIndex] = alongtrackiondrift; // positive along satellite velocity vector (approximate direction)
        ionDriftError[hmTimeIndex] = vionsError;
        viFlags[hmTimeIndex] = viFlag;
        ionDriftRaw[hmTimeIndex] = alongtrackiondrift;

        ionDensity[hmTimeIndex] = ni / 1e6; // /cm^3
        ionDensityError[hmTimeIndex] = niError;
        niFlags[hmTimeIndex] = niFlag;

        fpAreaOML[hmTimeIndex] = fpArea;
        rProbeOML[hmTimeIndex] = rProbe;

        iterationCount[hmTimeIndex] = iterations;
    }

    *numberOfSlidemEstimates = slidemEstimates;

    return;
}

void getTeVs(const char satellite, uint8_t **hmDataBuffers, long hmTimeIndex, double *te, uint32_t *teSource, double *vs, uint32_t *vsSource)
{
#if BLENDED_TE

    *te = TELEC();
    *teSource = LP_BLENDED_PROBE;

#else
    double tetmp = MISSING_TE_VALUE;
    uint32_t teSourcetmp = LP_NO_PROBE;

    if ((LPFLAG() & LP_TE_HGN_MASK) == 0 && (LPFLAG() & 0b11) != 0)
    {
        // Lomidze et al. 2021, Estimation of Ion Temperature Along the Swarm Satellite Orbits
        // Earth and Space Science e2021IEA001925
        switch(satellite)
        {
            case 'A':
                tetmp = 1.2844 * TEHGN() - 1083.0;
                break;
            case 'B':
                tetmp = 1.1626 * TEHGN() - 827.0;
                break;
            case 'C':
                tetmp = 1.2153 * TEHGN() - 916.0;
                break;
            default:
                tetmp = MISSING_TE_VALUE; 
                break;               
        }
        teSourcetmp = LP_HGN_PROBE;
    }
    else if ((LPFLAG() & LP_TE_LGN_MASK) == 0 && (LPFLAG() & 0b11) != 3)
    {
        switch(satellite)
        {
            case 'A':
                tetmp = 1.0 * TELGN() - 723.0;
                break;
            case 'B':
                tetmp = 1.0 * TELGN() - 698.0;
                break;
            case 'C':
                tetmp = 1.0 * TELGN() - 682.0;
                break;
            default:
                tetmp = MISSING_TE_VALUE; 
                break;               
        }
        teSourcetmp = LP_LGN_PROBE;
    }
    else 
    {
        tetmp = MISSING_TE_VALUE;
        teSourcetmp = LP_NO_PROBE;
    }
    *teSource = teSourcetmp;
    *te = tetmp;

#endif // BLENDED_TE

#if BLENDED_VS

    *vs = USC();
    *vsSource = LP_BLENDED_PROBE;

#else

    // Get VS
    if ((LPFLAG() & LP_VS_HGN_MASK) == 0 && (LPFLAG() & 0b11) != 0)
    {
        *vs = VSHGN();
        *vsSource = LP_HGN_PROBE;
    }
    else if ((LPFLAG() & LP_VS_LGN_MASK) == 0 && (LPFLAG() & 0b11) != 3)
    {
        *vs = VSLGN();
        *vsSource = LP_LGN_PROBE;
    }
    else 
    {
        *vs = MISSING_VS_VALUE;
        *vsSource = LP_NO_PROBE;
    }

#endif

    return;
}