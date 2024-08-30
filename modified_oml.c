/*

    SLIDEM Processor: modified_oml.c

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

double faceplateArea(double ni, double te, double phisc0, double mieff, double vionram, double faceplateVoltage, faceplateParams params)
{
    double areaModifier = params.areaModifier;
    double alpha = params.alpha;
    double beta = params.beta;
    double gamma = params.gamma;
    double m = mieff*SLIDEM_MAMU;
    double perimeter = 2.0*(SLIDEM_WFP + SLIDEM_HFP);
    double ageo = SLIDEM_WFP * SLIDEM_HFP;
    double lambdad = debyeLength(ni, te);

    double phisc = faceplateVoltage + phisc0; // Processing assumes faceplate potential is -3.5 V

    double delta = alpha * perimeter * lambdad / ageo * (1.0 - SLIDEM_QE * phisc / (0.5 * m * vionram*vionram) - beta * SLIDEM_QE * phisc / (SLIDEM_K * te) - gamma /(SLIDEM_QE * phisc) * SLIDEM_QE * SLIDEM_QE / (4. * M_PI * SLIDEM_EPS * lambdad));

    return (ageo * (1.0 + delta)) * (1.0 + areaModifier);

}

double probeRadius(double ni, double te, double phisc0, double mieff, double vionram, probeParams params)
{
    double radiusModifier = params.radiusModifier;
    double alpha = params.alpha;
    double beta = params.beta;
    double gamma = params.gamma;
    double zeta = params.zeta;
    double eta = params.eta;
    double m = mieff*SLIDEM_MAMU;
    double lambdad = debyeLength(ni, te);

    double phisc = phisc0;

    double delta = alpha * lambdad / SLIDEM_RP * (1.0 - beta * SLIDEM_QE * phisc / (0.5 * m * vionram*vionram) - gamma * SLIDEM_QE * phisc / (SLIDEM_K * te)) - zeta * phisc + eta;

    return (SLIDEM_RP * sqrt(1.0 - delta))*(1.0 + radiusModifier);
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

int loadModifiedOMLParams(faceplateParams * fpParams, probeParams * sphericalProbeParams)
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
    if (fscanf(configFP, "%lf %lf %lf %lf", &fpParams->areaModifier, &fpParams->alpha, &fpParams->beta, &fpParams->gamma) != 4)
    {
        fprintf(stdout, "Error reading faceplate OML parameters.\n");
        fclose(configFP);
        return MODIFIED_OML_ERROR_CONFIG_FILE_FACEPLATE_PARAMS;
    }
    if (fscanf(configFP, "%lf %lf %lf %lf %lf %lf", &sphericalProbeParams->radiusModifier, &sphericalProbeParams->alpha, &sphericalProbeParams->beta, &sphericalProbeParams->gamma, &sphericalProbeParams->zeta, &sphericalProbeParams->eta) != 6)
    {
        fprintf(stdout, "Error reading spherical probe OML parameters.\n");
        fclose(configFP);
        return MODIFIED_OML_ERROR_CONFIG_FILE_SPHERICAL_PROBE_PARAMS;
    }
    fclose(configFP);

    return MODIFIED_OML_ERROR_OK;
}