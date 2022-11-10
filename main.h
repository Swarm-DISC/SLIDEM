/*

    SLIDEM Processor: main.h

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

#ifndef MAIN_H
#define MAIN_H

// Adapted from the Swarm Thermal Ion Imager Cross-track Ion Drift processor source code

#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include <cdf.h>

#define TIME() ((double)*((double*)dataBuffers[0]+(timeIndex))) // time for dataBuffers[0]
#define FPTIME() ((double)*((double*)fpDataBuffers[0]+(fpTimeIndex))) // EXTD LP_FP time (16 Hz)
#define HMTIME() ((double)*((double*)hmDataBuffers[0]+(hmTimeIndex))) // EXTD LP_HM time (2 Hz)
#define VNECTIME() ((double)*((double*)vnecDataBuffers[0]+(vnecTimeIndex))) // VNEC time (1 Hz)
#define MAGTIME() ((double)*((double*)magDataBuffers[0]+(magTimeIndex))) // MAG time (1 Hz)

#define FPADDR(n, m, d) (((double*)fpDataBuffers[(n)]+(d*fpTimeIndex + m)))
#define FPMEAS(n, m, d) ((double)(*(FPADDR(n, m, d))))
#define FPCURRENT() (FPMEAS(1, 0, 1)) // Faceplate current, negative is ions, 16 Hz (nA)

#define HMADDR(n, m, d) (((double*)hmDataBuffers[(n)]+(d*hmTimeIndex + m)))
#define HMMEAS(n, m, d) ((double)(*(HMADDR(n, m, d))))
#define LAT() (HMMEAS(1, 0, 1)) // Geographic latitude (degrees)
#define LON() (HMMEAS(2, 0, 1)) // Geographic longitude (degress)
#define RADIUS() (HMMEAS(3, 0, 1)) // Swarm radial position ITRF (m)
#define HEIGHT() (HMMEAS(4, 0, 1)) // Swarm Height (m)
#define QDLAT() (HMMEAS(5, 0, 1)) // Quasi-dipole magnetic latitude (degrees)
#define MLAT() (HMMEAS(6, 0, 1)) // Magnetic APEX latitude (using this for invariant latitude for ion composition model CALION)
#define MLT() (HMMEAS(7, 0, 1)) // Magnetic local time (hours)
#define NI() (HMMEAS(8, 0, 1)) // Ni derived from ion admittance (EXTD dataset) (cm^-3)
#define TEHGN() (HMMEAS(9, 0, 1)) // LP EXTD Te high gain probe (K)
#define TELGN() (HMMEAS(10, 0, 1)) // LP EXTD Te low gain probe (K)
#define TELEC() (HMMEAS(11, 0, 1)) // LP EXTD T_elec blended from both probes (K)
#define VSHGN() (HMMEAS(12, 0, 1)) // LP EXTD Phi from high gain probe (V)
#define VSLGN() (HMMEAS(13, 0, 1)) // LP EXTD Phi from low gain probe (V)
#define USC() (HMMEAS(14, 0, 1)) // LP EXTD USC blended from both probes (V)
#define LPFLAG() (*((uint32_t*)hmDataBuffers[(n)]+hmTimeIndex))
#define VNECADDR(n, m, d) (((double*)vnecDataBuffers[(n)]+(d*vnecTimeIndex + m)))
#define VNECMEAS(n, m, d) ((double)(*(VNECADDR(n, m, d))))
#define VN() (VNECMEAS(1, 0, 1)) // satellite velocity north component (m/s)
#define VE() (VNECMEAS(2, 0, 1)) // satellite velocity east component (m/s)
#define VC() (VNECMEAS(3, 0, 1)) // satellite velocity centre component (m/s)

#define MAGADDR(n, m, d) (((double*)magDataBuffers[(n)]+(d*magTimeIndex + m)))
#define MAGMEAS(n, m, d) ((double)(*(MAGADDR(n, m, d))))
#define BN() (MAGMEAS(1, 0, 3)) // MAG north component (nT)
#define BE() (MAGMEAS(1, 1, 3)) // MAG east component (nT)
#define BC() (MAGMEAS(1, 2, 3)) // MAG centre component (nT)
#define MAGFLAGSB() (*((uint8_t*)magDataBuffers[2]+magTimeIndex))
#define MAGFLAGSQ() (*((uint8_t*)magDataBuffers[3]+magTimeIndex))


#endif // MAIN_H
