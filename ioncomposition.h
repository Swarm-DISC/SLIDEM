/*

    SLIDEM Processor: ioncomposition.h

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

#ifndef _IONCOMPOSITION_H
#define _IONCOMPOSITION_H

void ionCompositionIriDK(double heightKm, double solarZenithAngle, double latitude, double f10point7, double seasonalDecimalMonth,
	 double *ionDensities);

double ionEffectiveMassIriDK(double heightKm, double solarZenithAngle, double latitude, double f10point7, double seasonalDecimalMonth);

void ionCompositionIriTBT(double heightKm, double diplatitude, double invlatitude, double mlt, double f107Adj, int dayOfYear, double ionRelativeDensities[4]);

double ionEffectiveMassIriTBT(double heightKm, double diplatitude, double invlatitude, double mlt, double f107Ad, int dayOfYear);


#endif // _IONCOMPOSITION_H
