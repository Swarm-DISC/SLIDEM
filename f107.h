/*

    SLIDEM Processor: f107.h

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

#ifndef _F107_H
#define _F107_H

enum F107_ERRORS {
    F107_OK = 0,
    F107_ERROR_FILE = -1,
    F107_ERROR_UNAVAILABLE = -2,
    F107_ERROR_DAY_OF_YEAR = -3
};

// Using long to be consistent with CDF epoch parsing in slidem.c
int loadF107FromAscii(long year, long month, long day, double *f107daily, double *f10781daymean, double *f107yearmean);

int f107Adjusted(long year, long month, long day, double *f107);

#endif // _F107_H
