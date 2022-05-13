/*

    SLIDEM Processor: f107.c

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

#include "f107.h"

#include "slidem_settings.h"
#include "utilities.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

extern char infoHeader[50];

int loadF107FromAscii(long year, long month, long day, double *f107, double *f10781, double *f107year)
{
    char *home = getenv("HOME");
    char f107File[255];
    sprintf(f107File, "%s/bin/apf107.dat", home);
    int status = (int) F107_ERROR_UNAVAILABLE;

    FILE *f107FP = fopen(f107File, "r");
    if (f107FP == NULL)
    {
        fprintf(stdout, "Error opening F10.7 solar activity data file. Exiting.\n");
        return F107_ERROR_FILE;
    }

    int yy = year - 1900;
    if (yy >= 100)
    {
        yy -=100;
    }

    char buf[55] = {0};
    char cy[3] = {0};
    char cm[3] = {0};
    char cd[3] = {0};
    char cf107day[5] = {0};
    char cf10781daymean[5] = {0};
    char cf107yearmean[5] = {0};
    double f107day = 0.0;
    double f10781daymean = 0.0;
    double f107yearmean = 0.0;
    int conversions = 0;
    int y, m, d, dummy;
    double f1, f2, f3; // read buffers

    while((conversions = fscanf(f107FP, "%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%5lf%5lf%5lf\n", &y, &m, &d, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &f1, &f2, &f3)) != EOF)
    {
        if (y == yy && m == month && d == day)
        {
            f107day = f1;
            f10781daymean = f2;
            f107yearmean = f3;
            status = (int) F107_OK;
            break;
        }
    }

    fclose(f107FP);
    *f107 = f107day;
    *f10781 = f10781daymean;
    *f107year = f107yearmean;

    return status;

}

int f107Adjusted(long year, long month, long day, double *f107Adj)
{
    // Transcribed from IRI 2016 fortran
    int yday = 0;
    int res = 0;
    if ((res = dayOfYear(year, month, day, &yday)))
    {
        *f107Adj = 0.0;
        return res;
    }

    double f107daily = -1.0;
    double f10781daymean = -1.0;
    double f107yearmean = -1.0;
    res = loadF107FromAscii(year,month,day, &f107daily, &f10781daymean, &f107yearmean);
    if (res != F107_OK)
    {
        *f107Adj = 0.0;
        return F107_ERROR_UNAVAILABLE;
    }

	double eexc = 0.01675; // eccentricity Earth's orbit
	double amx = M_PI*(yday-3.0)/182.6;
	double radj = 1.0-eexc*(cos(amx)+eexc*(cos(2.0*amx)-1.0)/2.0);
	double f_adj = radj*radj;
	double pf107=(f107daily+f10781daymean)/2.;
	double pf107obs=pf107/f_adj;

    *f107Adj = pf107obs;

    return F107_OK;

}
