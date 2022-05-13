/*

    SLIDEM Processor: calion.h

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

#ifndef _CALION_H
#define _CALION_H

void calion(double diplatitude, double invlat, double mlt, double alt, int ddd, double f107Adjusted,double ionRelativeDensities[4]);

double invariantDipLatitude(double dipLatitude, double invLatitude);

double ionlow(double invdiplat, double mlt, double alt, int ddd,
	 double d[4][3][49], int ion);

double ionhigh(double invdiplat, double mlt, double alt, int ddd,
	 double d[4][3][49], int ion);



#endif // _CALION_H
