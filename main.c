/*

    SLIDEM Processor: main.c

    Copyright (C) 2023 Johnathan K Burchill

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

// SLIDEM - Swarm Ion Drift, Effective Mass and Ion Drift
// Operational processor 
// Author: Johnathan K. Burchill, University of Calgary
// Developed under contract to DTU Space under the auspices of the Swarm Data, Innovation and
// Science Cluster (Swarm DISC).


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>

#include "load_inputs.h"
#include "utilities.h"
#include "main.h"
#include "slidem_settings.h"
#include "downsample.h"
#include "interpolate.h"
#include "modified_oml.h"
#include "calculate_diplatitude.h"
#include "calculate_products.h"
#include "post_process.h"
#include "export_products.h"
#include "write_header.h"

#include "f107.h"
#include "load_satellite_velocity.h"

#include "cdf_attrs.h"
#include "cdf_vars.h"


#include <gsl/gsl_errno.h>

char infoHeader[50];

int main(int argc, char* argv[])
{

    time_t processingStartTime = time(NULL);

    fprintf(stdout, "SLIDEM Swarm Langmuir Probe Ion Drift, Density and Effective Mass processor.\n");

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--about") == 0)
        {
            fprintf(stdout, "SLIDEM Swarm Langmuir Probe Ion Drift, Density and Effective Mass processor, version %s.\n", SOFTWARE_VERSION);
            fprintf(stdout, "Copyright (C) 2023 Johnathan K Burchill\n");
            fprintf(stdout, "This program comes with ABSOLUTELY NO WARRANTY.\n");
            fprintf(stdout, "This is free software, and you are welcome to redistribute it\n");
            fprintf(stdout, "under the terms of the GNU General Public License.\n");
            exit(0);
        }
    }

    if (argc != 7)
    {
        fprintf(stdout, "SLIDEM processor called as:\n \"");
        for (int i = 0; i < argc; i++)
        {
            fprintf(stdout, "%s ", argv[i]);
        }
        fprintf(stdout, "\"\n");
        fprintf(stdout, "usage:\tslidem satellite yyyymmdd lpDirectory modDirectory magDirectory exportDirectory\n\t\tprocesses Swarm LP data to generate SLIDEM product for specified satellite and date.\n");
        fprintf(stdout, "\tslidem --about\n\t\tprints version and license information.\n");
        exit(1);
    }

    char * satelliteLetter = argv[1];
    char satellite = satelliteLetter[0];
    char * processingDate = argv[2];
    char *lppath = argv[3];
    char * modpath = argv[4];
    char *magpath = argv[5];
    char * exportDir = argv[6];

    long year, month, day;
    int valuesRead = sscanf(processingDate, "%4ld%2ld%2ld", &year, &month, &day);
    if (valuesRead != 3)
    {
        fprintf(stdout, "SLIDEM processor called as:\n \"%s %s %s %s %s\"\n Unable to parse date \"\". Exiting.\n", argv[0], argv[1], argv[2], argv[3], argv[4]);
        exit(1);
    }

    // set up info header
    sprintf(infoHeader, "SLIDEM %c%s %04ld-%02ld-%02ld: ", satellite, EXPORT_VERSION_STRING, year, month, day);

    char slidemFilename[CDF_PATHNAME_LEN+1];
    double beginTime;
    double endTime;
    if(constructExportFileName(satellite, year, month, day, exportDir, &beginTime, &endTime, slidemFilename))
    {
        fprintf(stdout, "%sCould not construct export filename. Exiting.\n", infoHeader);
        exit(1);
    }

    char fpFilename[FILENAME_MAX];
    if (getInputFilename(satellite, year, month, day, lppath, "LP_FP", fpFilename))
    {
        fprintf(stdout, "%sEXTD LP_FP input file is not available. Exiting.\n", infoHeader);
        exit(1);
    }

    // Confirm requested date has records. Abort otherwise.
    long numAvailableRecords = numberOfAvailableRecords(fpFilename);

    if (numAvailableRecords < (16 * SECONDS_OF_DATA_REQUIRED_FOR_PROCESSING))
    {
        fprintf(stdout, "%sLess than %.0f s of data available. Skipping this date.\n", infoHeader, (float)SECONDS_OF_DATA_REQUIRED_FOR_PROCESSING);
        exit(1);
    }

    char hmFilename[FILENAME_MAX];
    if (getInputFilename(satellite, year, month, day, lppath, "LP_HM", hmFilename))
    {
        fprintf(stdout, "%sEXTD LP_HM input file is not available. Exiting.\n", infoHeader);
        exit(1);
    }

    char magFilename[FILENAME_MAX];
    if (getInputFilename(satellite, year, month, day, magpath, "LR_1B", magFilename))
    {
        fprintf(stdout, "%sMAG LR_1B input file is not available. Exiting.\n", infoHeader);
        exit(1);
    }

    // get day of year for CALION ion composition model
    int yday = 0;
    if (dayOfYear(year, month, day, &yday))
    {
        fprintf(stdout, "%sUnable to calculate day of year from date. Exiting.\n", infoHeader);
        exit(1);
    }

    // Exit if SLIDEM CDF file exists.
    char slidemFullFilename[FILENAME_MAX];
    sprintf(slidemFullFilename, "%s.ZIP", slidemFilename);
    if (access(slidemFullFilename, F_OK) == 0)
    {
        fprintf(stdout, "%sSLIDEM CDF file exists. Skipping this date.\n", infoHeader);
        exit(1);
    }

    char modFilename[FILENAME_MAX];
    if (getInputFilename(satellite, year, month, day, modpath, "SC_1B", modFilename))
    {
        fprintf(stdout, "%sOPER MODx SC_1B input file is not available. Exiting.\n", infoHeader);
        exit(1);
    }
    long yearprev, monthprev, dayprev, hourprev, minuteprev, secondprev, msecprev;
    EPOCHbreakdown(beginTime - 86400000, &yearprev, &monthprev, &dayprev, &hourprev, &minuteprev, &secondprev, &msecprev);
    char modFilenamePrevious[FILENAME_MAX];
    if (getInputFilename(satellite, yearprev, monthprev, dayprev, modpath, "SC_1B", modFilenamePrevious))
    {
        sprintf(modFilenamePrevious, "%s", "<unavailable>");
    }

    // Exit if F10.7 is not available
    double f107Adj = 0.0;
    if (f107Adjusted(year, month, day, &f107Adj) != F107_OK)
    {
        fprintf(stdout, "%sF 10.7 is unavailable for this date. Check that your $HOME/bin/apf107.dat file is present and up to date. Skipping this date.\n", infoHeader);
        exit(1);
    }

    // config file for faceplate and spherical probe modified OML parameters
    faceplateParams fpParams;
    probeParams sphericalProbeParams;
    if (loadModifiedOMLParams(&fpParams, &sphericalProbeParams))
    {
        fprintf(stdout, "%sError loading Modified OML parameters. Exiting.\n", infoHeader);
        exit(1);
    }

    fprintf(stdout, "\n%s-------------------------------------------------\n", infoHeader);
    fprintf(stdout, "%s%s (%s)\n", infoHeader, SOFTWARE_VERSION_STRING, EXPORT_VERSION_STRING);

    // Processing info for logging from command line
    struct tm * pt = gmtime(&processingStartTime);
    fprintf(stdout, "%sProcessing date: UTC=%4d-%02d-%02dT%02d:%02d:%02d\n", infoHeader, pt->tm_year+1900, pt->tm_mon+1, pt->tm_mday, pt->tm_hour, pt->tm_min, pt->tm_sec);
    fprintf(stdout, "%sSLIDEM filename: %s.cdf\n", infoHeader, slidemFilename);
    fprintf(stdout, "%sFP filename: %s\n", infoHeader, fpFilename);
    fprintf(stdout, "%sHM filename: %s\n", infoHeader, hmFilename);
    fprintf(stdout, "%sMOD filename: %s\n", infoHeader, modFilename);
    fprintf(stdout, "%sMOD filename for previous day: %s\n", infoHeader, modFilenamePrevious);
    fprintf(stdout, "%sMAG filename: %s\n", infoHeader, magFilename);
    fprintf(stdout, "%sF10.7 adjusted for TBT composition model: %7.2f (apf107.dat file courtesy ECHAIM project at https://chain-new.chain-project.net/echaim_downloads/apf107.dat)\n", infoHeader, f107Adj);
    fprintf(stdout, "%sDay of year for TBT composition model: %3d\n", infoHeader, yday);
    if (MODIFIED_OML_GEOMETRIES)
    {
        fprintf(stdout, "%sUsing modified OML geometries\n", infoHeader);
        if (BLENDED_TE)
            fprintf(stdout, "%s  Te source: EXTD blended (no adjustment applied)\n", infoHeader);
        else
            fprintf(stdout, "%s  Te source: EXTD best probe (with Lomidze et al. (2021) adjustment)\n", infoHeader);
        if (BLENDED_VS)
            fprintf(stdout, "%s  Satellite potential source: EXTD blended\n", infoHeader);
        else
            fprintf(stdout, "%s  Satellite potential source: EXTD best probe\n, infoHeader", infoHeader);
        fprintf(stdout, "%s  Parameters:\n", infoHeader);
        fprintf(stdout, "%s   Faceplate: areaModifier=%f alpha=%f beta=%f gamma=%f\n", infoHeader, fpParams.areaModifier, fpParams.alpha, fpParams.beta, fpParams.gamma);
        fprintf(stdout, "%s   Spherical probe: radiusModifier=%f alpha=%f beta=%f gamma=%f zeta=%f eta=%f\n", infoHeader, sphericalProbeParams.radiusModifier, sphericalProbeParams.alpha, sphericalProbeParams.beta, sphericalProbeParams.gamma, sphericalProbeParams.zeta, sphericalProbeParams.eta);
    }

    CDFstatus status;

    // Turn off GSL failsafe error handler. We typically check the GSL return codes.
    gsl_set_error_handler_off();

    // load input data
    char *fpVariables[NUM_FP_VARIABLES] = {
        "Timestamp",
        "Current"
    };
    uint8_t * fpDataBuffers[NUM_FP_VARIABLES];
    for (uint8_t i = 0; i < NUM_FP_VARIABLES; i++)
    {
        fpDataBuffers[i] = NULL;
    }
    long nFp16HzRecs = 0;
    loadInputs(fpFilename, fpVariables, NUM_FP_VARIABLES, fpDataBuffers, &nFp16HzRecs);
    fflush(stdout);

    char *hmVariables[NUM_HM_VARIABLES] = {
        "Timestamp",
        "Latitude",
        "Longitude",
        "Radius",
        "Height",
        "Diplat",
        "MLat",
        "MLT",
        "n",
        "Te_hgn",
        "Te_lgn",
        "T_elec",
        "Vs_hgn",
        "Vs_lgn",
        "U_SC",
        "Flagbits"
    };
    uint8_t * hmDataBuffers[NUM_HM_VARIABLES];
    for (uint8_t i = 0; i < NUM_HM_VARIABLES; i++)
    {
        hmDataBuffers[i] = NULL;
    }
    long nHmRecs = 0;
    loadInputs(hmFilename, hmVariables, NUM_HM_VARIABLES, hmDataBuffers, &nHmRecs);
    // Convert heights from km to m
    // Ensure longitude is within the range -180 to +180
    for (long hmTimeIndex = 0; hmTimeIndex < nHmRecs; hmTimeIndex++)
    {
        ((double*)hmDataBuffers[4])[hmTimeIndex] = 1000. * HEIGHT();
        if (LON() > 180.0)
            ((double*)hmDataBuffers[2])[hmTimeIndex] = LON() - 360.0;
        if (LON() < -180.0)
            ((double*)hmDataBuffers[2])[hmTimeIndex] = LON() + 360.0;
    }

    // Magnetic field for dip latitude calculation
    char *magVariables[NUM_MAG_VARIABLES] = {
        "Timestamp",
        "B_NEC",
        "Flags_B",
        "Flags_q"
    };
    uint8_t * magDataBuffers[NUM_MAG_VARIABLES];
    for (uint8_t i = 0; i < NUM_MAG_VARIABLES; i++)
    {
        magDataBuffers[i] = NULL;
    }
    long nMagRecs = 0;
    loadInputs(magFilename, magVariables, NUM_MAG_VARIABLES, magDataBuffers, &nMagRecs);
    if (nMagRecs == 0)
    {
        fprintf(stdout, "%sUnable to load magnetic field. Skipping this date.\n", infoHeader);
        goto cleanup;

    }
    double * dipLat = (double*)malloc((size_t)(sizeof(double) * nMagRecs));
    long nDipLatRecs = nMagRecs;
    calculateDipLatitude(magDataBuffers, nMagRecs, dipLat);

    // Satellite velocity
    uint8_t * vnecDataBuffers[4];
    for (uint8_t i = 0; i < 4; i++)
    {
        vnecDataBuffers[i] = NULL;
    }
    long nVnecRecs = 0, nVnecRecsPrev = 0;
    if(loadSatelliteVelocity(modFilename, vnecDataBuffers, &nVnecRecs))
    {
        fprintf(stdout, "%sUnable to load satellite velocity. Skipping this date.\n", infoHeader);
        goto cleanup;
    }
    // Previous date
    uint8_t * vnecDataBuffersPrev[4];
    for (uint8_t i = 0; i < 4; i++)
    {
        vnecDataBuffersPrev[i] = NULL;
    }
    // Previous day used if available but not required, so do not exit if could not read velocities
    loadSatelliteVelocity(modFilenamePrevious, vnecDataBuffersPrev, &nVnecRecsPrev);
    if (nVnecRecsPrev > 0)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            vnecDataBuffersPrev[i] = (uint8_t*) realloc(vnecDataBuffersPrev[i], (size_t) sizeof(double)*(nVnecRecsPrev + nVnecRecs));
            memcpy(vnecDataBuffersPrev[i] + (size_t)(sizeof(double)*nVnecRecsPrev), vnecDataBuffers[i], (size_t)(sizeof(double)*nVnecRecs));
            free(vnecDataBuffers[i]);
            vnecDataBuffers[i] = vnecDataBuffersPrev[i];
        }
        nVnecRecs += nVnecRecsPrev;
    }

    // Update radius variable
    for (long hmTimeIndex = 0; hmTimeIndex < nHmRecs; hmTimeIndex++)
    {
        //(*((double*)hmDataBuffers[3]+(hmTimeIndex))) = RADIUS() * 1000.0; // m
    	// Radius is 0 in recent LP files. Temporary workaround:
        (*((double*)hmDataBuffers[3]+(hmTimeIndex))) = (6371.0 * 1000.0 + HEIGHT()); // m
    }
 
    // Number of records obtained for this date
    fprintf(stdout, "%sRead input data. FP: %ld s HM: %ld s VNEC: %ld s MAG: %ld s.\n", infoHeader, nFp16HzRecs / 16, nHmRecs / 2, nVnecRecs, nMagRecs);
    fflush(stdout);

    if (nHmRecs == 0 || nFp16HzRecs == 0 || nVnecRecs == 0)
    {
        fprintf(stdout, "%sError: one or more input files does not have records. Skipping this date.\n", infoHeader);
        fflush(stdout);
        goto cleanup;
    }

    // Downsample Faceplate data
    long nFpRecs = nFp16HzRecs;
    downSample(fpDataBuffers, NUM_FP_VARIABLES, &nFpRecs);

    double *fpCurrent = (double*) malloc((size_t) (nHmRecs * sizeof(double)));
    // If there are no measurements within 0.5 s of the HM input time, this sets fpCurrent to NaN.
    interpolateFpCurrent(fpDataBuffers, nFpRecs, hmDataBuffers, nHmRecs, fpCurrent);
    fprintf(stdout, "%sDownsampled and interpolated FP current to HM times.\n", infoHeader);

    double *fpVoltage = (double*) malloc((size_t) (nHmRecs * sizeof(double)));
    for (long i = 0; i < nHmRecs; i++)
    {
        //for now assume -3.5 V 
        fpVoltage[i] = FACEPLATE_VOLTAGE;
    }    

    // Interpolate satellite V NEC data
    double *vn = (double*) malloc((size_t) (nHmRecs * sizeof(double)));
    double *ve = (double*) malloc((size_t) (nHmRecs * sizeof(double)));
    double *vc = (double*) malloc((size_t) (nHmRecs * sizeof(double)));
    interpolateVNEC(vnecDataBuffers, nVnecRecs, hmDataBuffers, nHmRecs, vn, 1);
    interpolateVNEC(vnecDataBuffers, nVnecRecs, hmDataBuffers, nHmRecs, ve, 2);
    interpolateVNEC(vnecDataBuffers, nVnecRecs, hmDataBuffers, nHmRecs, vc, 3);
    fprintf(stdout, "%sInterpolated VNEC to HM times.\n", infoHeader);
    
    // Interpolate dip latitude to 2 Hz HM times
    double *dipLatitude = (double*) malloc((size_t) (nHmRecs * sizeof(double)));
    interpolateDipLatitude((double*)magDataBuffers[0], dipLat, nDipLatRecs, hmDataBuffers, nHmRecs, dipLatitude);
    fprintf(stdout, "%sInterpolated dip latitude to HM times.\n", infoHeader);
    
    // Calculate SLIDEM products
    double *ionEffectiveMass = (double*) malloc((size_t) (nHmRecs * sizeof(double)));
    double *ionDensity = (double*) malloc((size_t) (nHmRecs * sizeof(double)));
    double *ionDriftRaw = (double*) malloc((size_t) (nHmRecs * sizeof(double)));
    double *ionDrift = (double*) malloc((size_t) (nHmRecs * sizeof(double)));
    double *ionEffectiveMassError = (double*) malloc((size_t) (nHmRecs * sizeof(double)));
    double *ionDensityError = (double*) malloc((size_t) (nHmRecs * sizeof(double)));
    double *ionDriftError = (double*) malloc((size_t) (nHmRecs * sizeof(double)));
    double *fpAreaOML = (double*) malloc((size_t) (nHmRecs * sizeof(double)));
    double *rProbeOML = (double*) malloc((size_t) (nHmRecs * sizeof(double)));
    double *electronTemperature = (double*) malloc((size_t) (nHmRecs * sizeof(double)));
    double *spacecraftPotential = (double*) malloc((size_t) (nHmRecs * sizeof(double)));
    double *ionEffectiveMassTTS = (double*) malloc((size_t) (nHmRecs * sizeof(double)));
    uint32_t *mieffFlags = (uint32_t*) malloc((size_t) (nHmRecs * sizeof(uint32_t)));
    uint32_t *viFlags = (uint32_t*) malloc((size_t) (nHmRecs * sizeof(uint32_t)));
    uint32_t *niFlags = (uint32_t*) malloc((size_t) (nHmRecs * sizeof(uint32_t)));
    uint16_t *iterationCount = (uint16_t*) malloc((size_t) (nHmRecs * sizeof(uint16_t)));
    long numberOfSlidemEstimates = 0;

    calculateProducts(satellite, hmDataBuffers, fpCurrent, vn, ve, vc, dipLatitude, fpVoltage, f107Adj, yday, ionEffectiveMass, ionDensity, ionDriftRaw, ionDrift, ionEffectiveMassError, ionDensityError, ionDriftError, fpAreaOML, rProbeOML, electronTemperature, spacecraftPotential, ionEffectiveMassTTS, mieffFlags, viFlags, niFlags, iterationCount, nHmRecs, fpParams, sphericalProbeParams, &numberOfSlidemEstimates);
    fprintf(stdout, "%sCalculated %ld SLIDEM IDM products.\n", infoHeader, numberOfSlidemEstimates);

    if (POST_PROCESS_ION_DRIFT)
    {
        postProcessIonDrift(slidemFullFilename, satellite, hmDataBuffers, vn, ve, vc, fpCurrent, fpVoltage, fpAreaOML, rProbeOML, electronTemperature, spacecraftPotential, ionEffectiveMassTTS, ionDrift, ionDriftError, ionEffectiveMass, ionEffectiveMassError, ionDensity, ionDensityError, viFlags, mieffFlags, niFlags, iterationCount, fpParams, sphericalProbeParams, nHmRecs);
    }

    // Write CDF file
    status = exportProducts(slidemFilename, satellite, beginTime, endTime, hmDataBuffers, nHmRecs, vn, ve, vc, ionEffectiveMass, ionDensity, ionDriftRaw, ionDrift, ionEffectiveMassError, ionDensityError, ionDriftError, fpAreaOML, rProbeOML, electronTemperature, spacecraftPotential, ionEffectiveMassTTS, mieffFlags, viFlags, niFlags, fpFilename, hmFilename, modFilename, modFilenamePrevious, magFilename, nVnecRecsPrev);

    if (status != CDF_OK)
    {
        fprintf(stdout, "%sCDF export failed. Not generating metainfo.\n", infoHeader);
        goto cleanup;
    }

    // Write Header file for L2 archiving
    time_t processingStopTime = time(NULL);
    long hmTimeIndex = 0;
    double firstMeasurementTime = HMTIME();
    hmTimeIndex = nHmRecs-1;
    double lastMeasurementTime = HMTIME();
    status = writeSlidemHeader(slidemFilename, fpFilename, hmFilename, modFilename, modFilenamePrevious, magFilename, processingStartTime, firstMeasurementTime, lastMeasurementTime, nVnecRecsPrev);

    if (status != HEADER_OK)
    {
        fprintf(stdout, "%sError writing HDR file.\n", infoHeader);
        goto cleanup;        
    }

    // Archive the CDF and HDR files in a ZIP file
    int sysStatus = system(NULL);
    if (sysStatus == 0)
    {
        fprintf(stderr, "%sSystem shell call not available. Not archiving CDF.\n", infoHeader);
        goto cleanup;
    }
    sysStatus = system("zip -q 1 > /dev/null");
    if (WIFEXITED(sysStatus) && WEXITSTATUS(sysStatus) == 12)
    {
        char command[5*FILENAME_MAX + 100];
        sprintf(command, "zip -Z store -q -r -j %s.ZIP %s.HDR %s.cdf && rm %s.HDR %s.cdf", slidemFilename, slidemFilename, slidemFilename, slidemFilename, slidemFilename);
        sysStatus = system(command);
        if (WIFEXITED(sysStatus) && (WEXITSTATUS(sysStatus) == 0))
        {
            fprintf(stdout, "%sStored HDR and CDF files in %s.ZIP\n", infoHeader, slidemFilename);
        }
        else
        {
            fprintf(stderr, "%sFailed to archive HDR and CDF files.\n", infoHeader);
        }
    }
    else
    {
        fprintf(stderr, "zip is unusable. Not archiving CDF.\n");
    }



cleanup:
    fflush(stdout);

    freeMemory(fpDataBuffers, hmDataBuffers, vnecDataBuffers, magDataBuffers, fpCurrent, vn, ve, vc, dipLat, dipLatitude, ionEffectiveMass, ionDensity, ionDriftRaw, ionDrift, ionEffectiveMassError, ionDensityError, ionDriftError, fpAreaOML, rProbeOML, electronTemperature, spacecraftPotential, fpVoltage, ionEffectiveMassTTS, mieffFlags, viFlags, niFlags, iterationCount);

    return 0;
}
