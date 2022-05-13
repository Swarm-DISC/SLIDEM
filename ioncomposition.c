
/*

    SLIDEM Processor: ioncomposition.c

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

// Based on IRI-2016 functions. See IRI2016-License.txt.
// Adapted from initial transription from fortran to C using f2c 
// from http://www.netlib.org/f2c/index.html
// "A Fortran to C Converter", S. I. Feldman, ACM SIGPLAN Fortran Forum, vol. 9, issue 2, p. 21–22 (1990).



#include "ioncomposition.h"

#include "calion.h"

#include <stdio.h>

#include <gsl/gsl_math.h>
#include <gsl/gsl_sf_exp.h>
#include <math.h>

void ionCompositionIriDK(double heightKm, double solarZenithAngle, double latitude, double f10point7, double seasonalDecimalMonth,
	 double *ionDensities)
{

	// From IRI 2016 old topside ion composition model (defaults to off in favour of Truhlik et al. 2015 model)
	// A.D. Danilov and A.P. Yaichnikov, A New Model of the Ion
	// Composition at 75 to 1000 km for IRI, Adv. Space Res. 5, #7, 
	// 75-79, 107-108, 1985
	//     heightKm       altitude in km 
	//     solarZenithAngle      solar zenith angle in degrees 
	//     latitude      latitude in degrees (same result for latitude and -latitude) 
	//     f10point7      10.7cm solar radio flux 
	//     seasonalDecimalMonth       seasonal decimal month 
	// 		(Northern Hemisphere January 15 is seasonalDecimalMonth=1.5 and so is Southern Hemisphere July 15) 
	//     ionDensities(0)   H+  relative density in percent 
	//     ionDensities(1)   O+  relative density in percent 
	//     ionDensities(2)   N+  relative density in percent 
	//     ionDensities(3)   He+ relative density in percent 
	// --------------------------------------------------------------- 


    double po[30] = { 0.0,0.0,0.0,0.0,98.5,0.0,0.0,
	    0.0,0.0,320.0,0.0,0.0,0.0,0.0,-2.59e-4,2.79e-4,-.00333,
	    -.00352,-.00516,-.0247,0.0,0.0,0.0,0.0,-2.5e-6,.00104,
	    -1.79e-4,-4.29e-5,1.01e-5,-.00127 };
    double ph[30] = { -4.97e-7,-.121,-.131,0.0,
	    98.1,355.0,-191.0,-127.0,0.0,2040.0,0.0,0.0,0.0,0.0,-4.79e-6,
	    -2e-4,5.67e-4,2.6e-4,0.0,-.00508,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
	    0.0,0.0,0.0 };
    double pn[30] = { .76,-5.62,-4.99,0.0,5.79,
	    83.0,-369.0,-324.0,0.0,593.0,0.0,0.0,0.0,0.0,-6.3e-5,-.00674,
	    -.00793,-.00465,0.0,-.00326,0.0,0.0,0.0,0.0,-1.17e-5,.00488,
	    -.00131,-7.03e-4,0.0,-.00238 };
    double phe[30] = { -.895,6.1,5.39,0.0,8.01,0.0,
	    0.0,0.0,0.0,1200.0,0.0,0.0,0.0,0.0,-1.04e-5,.0019,9.53e-4,
	    .00106,0.0,-.00344,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 };

    /* Local variables */
    double latitudeRadian;
    double p[5][6][4];
	double totalIonDensity;
	double z;
	double cm[4];
	double hm[4];
	double hx;
	double alh[4];
	double all[4];
	double arg;
	double var[6];
	double beth[4];
	double betl[4];

	double deg2rad = M_PI / 180.0;
	double argmax = 90.0; // 80 or 88 in Fortran version. Could be larger with GSL. GSL does not have a definition of argmax.

    z = solarZenithAngle * deg2rad;
    latitudeRadian = latitude * deg2rad;
	for (int j = 0; j < 6; ++j) {
	    for (int i = 0; i < 5; i++) {
			p[i][j][0] = po[i + j * 5];
			p[i][j][1] = pn[i + j * 5];
			p[i][j][2] = phe[i + j * 5];
			p[i][j][3] = ph[i + j * 5];
		}
    }
    totalIonDensity = 0.0;
    for (int ion = 0; ion < 4; ion++) {
		cm[ion] = p[0][0][ion] * cos(z) + p[1][0][ion] * cos(latitudeRadian) + p[2][0][ion] * cos((300.0 - f10point7) * 0.013) + p[3][0][ion] * cos((seasonalDecimalMonth - 6.0) * 0.52) + p[4][0][ion];
		hm[ion] = p[0][1][ion] * cos(z) + p[1][1][ion] * cos(latitudeRadian) + p[2][1][ion] * cos((300.0 - f10point7) * 0.013) + p[3][1][ion] * cos((seasonalDecimalMonth - 6.0) * 0.52) + p[4][1][ion];
		all[ion] = p[0][2][ion] * cos(z) + p[1][2][ion] * cos(latitudeRadian) + p[2][2][ion] * cos((300.0 - f10point7) * 0.013) + p[3][2][ion] * cos((seasonalDecimalMonth - 6.0) * 0.52) + p[4][2][ion];
		betl[ion] = p[0][3][ion] * cos(z) + p[1][3][ion] * cos(latitudeRadian) + p[2][3][ion] * cos((300.0 - f10point7) * 0.013) + p[3][3][ion] * cos((seasonalDecimalMonth - 6.0) * 0.52) + p[4][3][ion];
		alh[ion] = p[0][4][ion] * cos(z) + p[1][4][ion] * cos(latitudeRadian) + p[2][4][ion] * cos((300.0 - f10point7) * 0.013) + p[3][4][ion] * cos((seasonalDecimalMonth - 6.0) * 0.52) + p[4][4][ion];
		beth[ion] = p[0][5][ion] * cos(z) + p[1][5][ion] * cos(latitudeRadian) + p[2][5][ion] * cos((300.0 - f10point7) * 0.013) + p[3][5][ion] * cos((seasonalDecimalMonth - 6.0) * 0.52) + p[4][5][ion];
		hx = heightKm - hm[ion];
//		fprintf(stdout, "ion: %d: cm: %e hm: %e all: %e betl: %e alh: %e beth: %e hx %e\n", ion+1, cm[ion], hm[ion], all[ion], betl[ion], alh[ion], beth[ion], hx);
		ionDensities[ion] = 0.0;
		if (hx <= 0.0) {
			arg = hx * (hx * all[ion] + betl[ion]);
		} else {
			arg = hx * (hx * alh[ion] + beth[ion]);
		}
		if (arg > -argmax) {
			ionDensities[ion] = cm[ion] * exp(arg);
		}
		if (ionDensities[ion] < cm[ion] * 0.005) {
			ionDensities[ion] = 0.0;
		}
		if (ionDensities[ion] > cm[ion]) {
			ionDensities[ion] = cm[ion];
		}
		totalIonDensity += ionDensities[ion];
    }
    for (int ion = 0; ion < 4; ++ion) {
		if (totalIonDensity > 0.0)
		{
			ionDensities[ion] = ionDensities[ion] / totalIonDensity * 100.0;
		}
		else
		{
			ionDensities[ion] = 0.0;
		}
    }
}


double ionEffectiveMassIriDK(double heightKm, double solarZenithAngle, double latitude, double f10point7, double seasonalDecimalMonth)
{
	double densities[4] = {0.};

	ionCompositionIriDK(heightKm, solarZenithAngle, latitude, f10point7, seasonalDecimalMonth, densities);
	double masses[4] = {16., 14., 4., 1.};
	double total = 0.0;
	double meanReciprocalMass = 0.0;
	double mieff = 0.0;
	double ni = 0.0;
	for (int i = 0; i < 4; i++)
	{
		ni = densities[i];
		if (ni >= 0.0)
		{
			total += ni;
			meanReciprocalMass += densities[i] / masses[i];
		}
	}
	if (total > 0)
	{
		meanReciprocalMass /= total;
		mieff = 1.0 / meanReciprocalMass;
	}
	// Otherwise mieff = 0.0, an unphysical value which can be flagged
	return mieff;
}

void ionCompositionIriTBT(double heightKm, double diplatitude, double invlatitude, double mlt, double f107Adj, int dayOfYear, double ionRelativeDensities[4])
{
	calion(diplatitude, invlatitude, mlt, heightKm, dayOfYear, f107Adj, ionRelativeDensities);
	return;
}

double ionEffectiveMassIriTBT(double heightKm, double diplatitude, double invlatitude, double mlt, double f107Adj, int dayOfYear)
{
	double densities[4] = {0.};

	ionCompositionIriTBT(heightKm, diplatitude, invlatitude, mlt, f107Adj, dayOfYear, densities);
	double masses[4] = {16., 14., 4., 1.};
	double total = 0.0;
	double meanReciprocalMass = 0.0;
	double mieff = 0.0;
	double ni = 0.0;
	for (int i = 0; i < 4; i++)
	{
		ni = densities[i];
		if (ni >= 0.0)
		{
			total += ni;
			meanReciprocalMass += densities[i] / masses[i];
		}
	}
	if (total > 0)
	{
		meanReciprocalMass /= total;
		mieff = 1.0 / meanReciprocalMass;
	}
	// Otherwise mieff = 0.0, an unphysical value which can be flagged
	return mieff;
}
