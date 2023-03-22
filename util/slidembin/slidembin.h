/*

    SLIDEM Processor: util/slidembin/slidembin.h

    Copyright (C) 2023  Johnathan K Burchill

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

#ifndef _SLIDEMBIN_H
#define _SLIDEMBIN_H

#include "statistics.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include <fts.h>
#include <cdf.h>

#define SOFTWARE_VERSION "1.0"
#define SOFTWARE_VERSION_STRING "slidembin 2023-03-21"

#define NUM_DATA_VARIABLES 7

typedef struct processingParameters
{
    int nOptions;
    char satelliteLetter;

    bool verbose;
    bool showFileProgress;

    char *cdfDirectory;
    char *inputFile;

    char *parameter;
    char *statistic;

    long nRecords;
    double *time;
    double *qdlat;
    double *mlt;
    double *values;
    uint32_t *flags;

    BinningState binningState;

    char *firstTimeString;
    char *lastTimeString;
    double firstTime;
    double lastTime;
    bool processAllSpaceSeries;

    long nFiles;

    uint32_t flagIgnoreMask;
    uint32_t positiveFlagMask;
    bool flagMaskIsAnd;
    bool flagRaisedIsGood;

} ProcessingParameters;

void usage(char *name);
void about(void);
void parseCommandLine(ProcessingParameters *params, int argc, char *argv[]);
bool fileMatch(FTSENT *e, ProcessingParameters *params);
int processFile(ProcessingParameters *params);
int loadSlidemData(ProcessingParameters *params);
CDFstatus loadCdfVariable(CDFid cdfId, char *variable, void **mem, long *nRecords);

void printQualityFlagTable(void);

#endif // _SLIDEMBIN_H
