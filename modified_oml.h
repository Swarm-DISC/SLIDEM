/*

    SLIDEM Processor: modified_oml.h

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

#ifndef _MODIFIED_OML
#define _MODIFIED_OML

typedef struct {
    double radiusModifier;
    double alpha;
    double bravo;
    double charlie;
} probeParams;

double probeRadius(double ni, double te, double phiSc, double mieff, double vionram, probeParams params, const char satellite);
double debyeLength(double ni, double te);

enum MODIFIED_OML_ERRORS {
    MODIFIED_OML_ERROR_OK = 0,
    MODIFIED_OML_ERROR_CONFIG_FILE = -1,
    MODIFIED_OML_ERROR_CONFIG_FILE_FACEPLATE_PARAMS = -2,
    MODIFIED_OML_ERROR_CONFIG_FILE_SPHERICAL_PROBE_PARAMS = -3
};

int loadModifiedOMLParams(probeParams * sphericalProbeParams);


#endif // _MODIFIED_OML
