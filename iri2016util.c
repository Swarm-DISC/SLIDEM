/*

    SLIDEM Processor: iri2016util.c

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

// Based on IRI-2016 functions in irifun.F. See IRI2016-License.txt.
// Transcribed using f2c from http://www.netlib.org/f2c/index.html
// "A Fortran to C Converter", S. I. Feldman, ACM SIGPLAN Fortran Forum, vol. 9, issue 2, p. 21–22 (1990).

#include "iri2016util.h"

#include <math.h>


double eptr(double x, double sc, double hx)
{
    double ret_val;
    double d1;
    double argmax = 88.0;

/* --------------------------------------------------------- TRANSITION */
    d1 = (x - hx) / sc;
    if (fabs(d1) < argmax)
    {
        ret_val = log(exp(d1) + 1.);
    }
    else if (d1 > 0.)
    {
    	ret_val = d1;
    }
    else
    {
	    ret_val = 0.;
    }
    return ret_val;
}

int spharm_ik(double *coeffs, int l, int m, double colat, double az)
{
    int k;
    double x, y;
    int mt;
    int n;
    double caz, saz;

/* ------------------------------------------------------------------------------------ */
/* CALCULATES THE COEFFICIENTS OF THE SPHERICAL HARMONIC */
/* FROM IRI 95 MODEL */
/* NOTE: COEFFICIENTS CORRESPONDING TO COS, SIN SWAPPED!!! */
/* ------------------------------------------------------------------------------------ */
 
    coeffs[0] = 1.;
    k = 2;
    x = cos(colat);
    coeffs[k-1] = x;
    k++;

    for (int i = 2; i <=l; i++)
    {
        coeffs[k-1] = ((2.0*(double)i - 1.0)*x*coeffs[k-2] - ((double)i-1.0)*coeffs[k-3])/((double) i);
        k++;
    }
    y = sin(colat);
    for (int mt = 1; mt <= m; ++mt) {
        caz = cos(mt * az);
        saz = sin(mt * az);
        coeffs[k-1] = pow(y, (double)mt);
        k++;
        if (mt == l) {
            goto L16;
        }
        coeffs[k-1] = coeffs[k - 1 - 1] * x * (2.0 * (double)mt + 1.0);
        k++;
        if (mt + 1 == l) {
            goto L16;
        }
        for (int i = mt + 2; i <= l; ++i) {
            coeffs[k-1] = ((2.0*(double)i - 1.0) * x * coeffs[k - 1 - 1] - (double)(i + mt - 1) * coeffs[k - 2 - 1]) / ((double)(i - mt));
            k++;
        }
    L16:
        n = l - mt + 1;
        for (int i = 1; i <= n; ++i) {
            coeffs[k - 1] = coeffs[k - n - 1] * saz;
            coeffs[k - n - 1] *= caz;
            k++;
        }
    }
    return 0;
}


