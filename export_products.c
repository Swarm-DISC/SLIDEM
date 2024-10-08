/*

    SLIDEM Processor: export_products.c

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

// Adapted from the Swarm Thermal Ion Imager Cross-track Ion Drift processor source code

#include "export_products.h"

#include "main.h"
#include "slidem_settings.h"
#include "cdf_vars.h"
#include "cdf_attrs.h"
#include "utilities.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <cdf.h>


extern char infoHeader[50];

CDFstatus exportProducts(const char *slidemFilename, char satellite, double beginTime, double endTime, uint8_t **hmDataBuffers, long nHmRecs, double *vn, double *ve, double *vc, double *ionEffectiveMass, double *ionDensity, double *ionDriftRaw, double *ionDrift, double *ionEffectiveMassError, double *ionDensityError, double *ionDriftError, double *fpAreaOML, double *rProbeOML, double *electronTemperature, double *spacecraftPotential, double *ionEffectiveMassTTS, uint32_t *mieffFlags, uint32_t *viFlags, uint32_t *niFlags, const char *fpFilename, const char *hmFilename, const char *modFilename, const char *modFilenamePrevious, const char *magFilename, long nVnecRecsPrev)
{
    long hmTimeIndex = 0;
    beginTime = HMTIME();
    hmTimeIndex = nHmRecs - 1;
    endTime = HMTIME();

    CDFstatus status = CDF_OK;

    status = exportSlidemCdf(slidemFilename, satellite, EXPORT_VERSION_STRING, hmDataBuffers, nHmRecs, vn, ve, vc, ionEffectiveMass, ionDensity, ionDriftRaw, ionDrift, ionEffectiveMassError, ionDensityError, ionDriftError, fpAreaOML, rProbeOML, electronTemperature, spacecraftPotential, ionEffectiveMassTTS, mieffFlags, viFlags, niFlags, fpFilename, hmFilename, modFilename, modFilenamePrevious, magFilename, nVnecRecsPrev);
    if (status != CDF_OK)
    {
        return status;
    }

    double minutesExported = (endTime - beginTime)/1000./60.;

    // report
    fprintf(stdout, "%sExported ~%.0f orbits (%ld 2 Hz records) of SLIDEM IDM data. %.1f%% coverage.\n", infoHeader, minutesExported/94., nHmRecs, minutesExported/1440.0*100.0);

    return status;
}

CDFstatus exportSlidemCdf(const char *slidemFilename, const char satellite, const char *exportVersion, uint8_t **hmDataBuffers, long nHmRecs, double *vn, double *ve, double *vc, double *ionEffectiveMass, double *ionDensity, double *ionDriftRaw, double *ionDrift, double *ionEffectiveMassError, double *ionDensityError, double *ionDriftError, double *fpAreaOML, double *rProbeOML, double *electronTemperature, double *spacecraftPotential, double *ionEffectiveMassTTS, uint32_t *mieffFlags, uint32_t *viFlags, uint32_t *niFlags, const char *fpFilename, const char *hmFilename, const char *modFilename, const char *modFilenamePrevious, const char *magFilename, long nVnecRecsPrev)
{

    fprintf(stdout, "%sExporting SLIDEM IDM data.\n", infoHeader);

    CDFid exportCdfId;
    CDFstatus status = CDF_OK;
    status = CDFcreateCDF((char *)slidemFilename, &exportCdfId);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        goto cleanup;
    }
    else
    {
        // velocity is a 1D variable (scalars are 0D in CDF parlance), per request of DTU
        double * vnec = malloc((size_t) (nHmRecs * 3 * sizeof(double)));
        if (vnec == NULL)
        {
            fprintf(stdout, "%s could not allocate memory to store VNEC.\n", infoHeader);
            status = EXPORT_MEM;
            goto cleanup;
        }
        for (long hmTimeIndex = 0; hmTimeIndex < nHmRecs; hmTimeIndex++)
        {
            vnec[3*hmTimeIndex] = vn[hmTimeIndex];
            vnec[3*hmTimeIndex + 1] = ve[hmTimeIndex];
            vnec[3*hmTimeIndex + 2] = vc[hmTimeIndex];
        }

        // export fpVariables
        createVarFrom1DVar(exportCdfId, "Timestamp", CDF_EPOCH, 0, nHmRecs-1, hmDataBuffers[0]);
        createVarFrom1DVar(exportCdfId, "Latitude", CDF_REAL8, 0, nHmRecs-1, hmDataBuffers[1]);
        createVarFrom1DVar(exportCdfId, "Longitude", CDF_REAL8, 0, nHmRecs-1, hmDataBuffers[2]);
        createVarFrom1DVar(exportCdfId, "Radius", CDF_REAL8, 0, nHmRecs-1, hmDataBuffers[3]);
        createVarFrom1DVar(exportCdfId, "Height", CDF_REAL8, 0, nHmRecs-1, hmDataBuffers[4]);
        createVarFrom1DVar(exportCdfId, "QDLatitude", CDF_REAL8, 0, nHmRecs-1, hmDataBuffers[5]);
        createVarFrom1DVar(exportCdfId, "MLT", CDF_REAL8, 0, nHmRecs-1, hmDataBuffers[7]);
        createVarFrom2DVar(exportCdfId, "V_sat_nec", CDF_REAL8, 0, nHmRecs-1, vnec, 3);
        createVarFrom1DVar(exportCdfId, "M_i_eff", CDF_REAL8, 0, nHmRecs-1, ionEffectiveMass);
        createVarFrom1DVar(exportCdfId, "M_i_eff_err", CDF_REAL8, 0, nHmRecs-1, ionEffectiveMassError);
        createVarFrom1DVar(exportCdfId, "M_i_eff_Flags", CDF_UINT4, 0, nHmRecs-1, mieffFlags);
        createVarFrom1DVar(exportCdfId, "M_i_eff_tbt_model", CDF_REAL8, 0, nHmRecs-1, ionEffectiveMassTTS);
        createVarFrom1DVar(exportCdfId, "V_i", CDF_REAL8, 0, nHmRecs-1, ionDrift);
        createVarFrom1DVar(exportCdfId, "V_i_err", CDF_REAL8, 0, nHmRecs-1, ionDriftError);
        createVarFrom1DVar(exportCdfId, "V_i_Flags", CDF_UINT4, 0, nHmRecs-1, viFlags);
        createVarFrom1DVar(exportCdfId, "V_i_raw", CDF_REAL8, 0, nHmRecs-1, ionDriftRaw);
        createVarFrom1DVar(exportCdfId, "N_i", CDF_REAL8, 0, nHmRecs-1, ionDensity);
        createVarFrom1DVar(exportCdfId, "N_i_err", CDF_REAL8, 0, nHmRecs-1, ionDensityError);
        createVarFrom1DVar(exportCdfId, "N_i_Flags", CDF_UINT4, 0, nHmRecs-1, niFlags);
        createVarFrom1DVar(exportCdfId, "A_fp", CDF_REAL8, 0, nHmRecs-1, fpAreaOML);
        createVarFrom1DVar(exportCdfId, "R_p", CDF_REAL8, 0, nHmRecs-1, rProbeOML);
        createVarFrom1DVar(exportCdfId, "T_e", CDF_REAL8, 0, nHmRecs-1, electronTemperature);
        createVarFrom1DVar(exportCdfId, "Phi_sc", CDF_REAL8, 0, nHmRecs-1, spacecraftPotential);

        // add attributes
        long hmTimeIndex = 0;
        double minTime = HMTIME();
        hmTimeIndex = nHmRecs - 1;
        double maxTime = HMTIME();

        char cdfFilename[FILENAME_MAX];
        snprintf(cdfFilename, FILENAME_MAX - 4, "%s.cdf", slidemFilename); 

        addAttributes(exportCdfId, SOFTWARE_VERSION_STRING, satellite, exportVersion, minTime, maxTime, cdfFilename, fpFilename, hmFilename, modFilename, modFilenamePrevious, magFilename, nVnecRecsPrev);

        fprintf(stdout, "%sExported %ld records to %s\n", infoHeader, nHmRecs, cdfFilename);
        fflush(stdout);
        status = CDF_OK;

    }

cleanup:
    closeCdf(exportCdfId);
    return status;

}
