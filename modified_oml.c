/*

    SLIDEM Processor: modified_oml.c

    Copyright (C) 2024  Johnathan K Burchill

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

#include "modified_oml.h"

#include "slidem_settings.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gsl/gsl_math.h>

// Empirical forumlae from 
//   Resendiz Lira and Marchand (2021), Simulation inference of plasma 
//     parameters from langmuir probe measurements, Earth and Space Science, 8(3), e2020EA001344
// and 
//   Lira et al. (2019), Determination of Swarm front plate's effective cross 
//     section from kinetic simulations, IEEE Transactions on plasma science, 47(8), 3667--3672.

extern char infoHeader[50];

double probeRadius(double ni, double te, double phisc0, double mieff, double vionram, probeParams params, const char satellite)
{
    double radiusModifier = params.radiusModifier;
    double m = mieff*SLIDEM_MAMU;
    double lambdad = debyeLength(ni, te);

    double phisc = phisc0;

    double a1 = params.alpha;
    if (satellite == 'B')
        a1 = params.bravo;
    else if (satellite == 'C')
        a1 = params.charlie;

    return (SLIDEM_RP * sqrt(1.0 + a1))*(1.0 + radiusModifier);
}

double debyeLength(double ni, double te)
{
    double arg = SLIDEM_EPS * SLIDEM_K * te / (ni * SLIDEM_QE * SLIDEM_QE);
    double length;
    if (arg > 0.)
        length = sqrt(arg);
    else
        length = GSL_NAN;

    return length;
}

int loadModifiedOMLParams(probeParams * sphericalProbeParams)
{
    char *home = getenv("HOME");
    char configFile[255];
    sprintf(configFile, "%s/.slidem_modified_oml_configrc_%s", home, EXPORT_VERSION_STRING);
    FILE *configFP = fopen(configFile, "r");
    if (configFP == NULL)
    {
        fprintf(stdout, "Error opening modified OML parameter file. Exiting.\n");
        return MODIFIED_OML_ERROR_CONFIG_FILE;
    }
    if (fscanf(configFP, "%lf %lf %lf %lf", &sphericalProbeParams->radiusModifier, &sphericalProbeParams->alpha, &sphericalProbeParams->bravo, &sphericalProbeParams->charlie) != 4)
    {
        fprintf(stdout, "Error reading spherical probe OML parameters.\n");
        fclose(configFP);
        return MODIFIED_OML_ERROR_CONFIG_FILE_SPHERICAL_PROBE_PARAMS;
    }
    fclose(configFP);

    return MODIFIED_OML_ERROR_OK;
}
