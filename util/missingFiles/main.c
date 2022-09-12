/*

    SLIDEM Processor: util/missingFiles/main.c

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

#include <stdio.h>

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <time.h>
#include <fts.h>

#define SOFTWARE_VERSION "1.0"

enum STATUS 
{
	STATUS_OK = 0,
	STATUS_PERMISSION,
	STATUS_MEM
};

int printMissingInputFiles(const char satelliteLetter, char *startDate, char *endDate, const char *dataset);

int dayCount(char *startDate, char *endDate);
int initDates(char **dates, char *startDate, char *endDate);

int main(int argc, char *argv[])
{

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--about") == 0)
        {
            fprintf(stdout, "syncInputs version %s.\n", SOFTWARE_VERSION);
            fprintf(stdout, "Copyright (C) 2022  Johnathan K Burchill\n");
            fprintf(stdout, "This program comes with ABSOLUTELY NO WARRANTY.\n");
            fprintf(stdout, "This is free software, and you are welcome to redistribute it\n");
            fprintf(stdout, "under the terms of the GNU General Public License.\n");
            exit(0);
        }
    }

	if (argc !=  5)
	{
		printf("usage:\t%s <satellite> <MAG|MOD|LP_HM|LP_FP> <start> <end> lists the missing input files for the specified satellite, file type, and date range.\n", argv[0]);
		printf("\t%s --about\n\t\tprints copyright and license information.\n", argv[0]);
		exit(0);
	}

	char *satLetter = argv[1];
	char *dataset = argv[2];
	char *startDate = argv[3];
	char *endDate = argv[4];

	printMissingInputFiles(satLetter[0], startDate, endDate, dataset);

	return 0;

}



int printMissingInputFiles(const char satelliteLetter, char *startDate, char *endDate, const char *dataset)
{
	char *searchPath[2] = {NULL, NULL};
    searchPath[0] = ".";

	FTS * fts = fts_open(searchPath, FTS_PHYSICAL | FTS_NOCHDIR, NULL);	
	if (fts == NULL)
	{
		printf("Could not open directory %s for reading.", searchPath[0]);
		return STATUS_PERMISSION;
	}

	int start = atoi(startDate);
	int end = atoi(endDate);
	int days = end - start + 1;

	// int days = dayCount(startDate, endDate);

	char **dates = NULL;
	dates = malloc(days * sizeof(char*));
	if (dates == NULL)
	{
		return STATUS_MEM;
	}


	for (int i = 0; i < days; i++)
	{
		dates[i] = "\0";
	}

	initDates(dates, startDate, endDate);

	FTSENT * f = fts_read(fts);

	int index = 0;
	while(f != NULL)
	{
        // Most Swarm CDF file names have a length of 59 characters. The MDR_MAG_LR files have a lend of 70 characters.
        // The MDR_MAG_LR files have the same filename structure up to character 55.
		if ((strlen(f->fts_name) == 59 || strlen(f->fts_name) == 70) && *(f->fts_name+11) == satelliteLetter && strncmp(f->fts_name+13, dataset, 5) == 0)
		{
            char fdate[9] = { 0 };
            strncpy(fdate, f->fts_name + 19, 8);
            index = atol(fdate) - start;
			if (index >= 0 && index < days)
			{
				dates[index] = "\0";
			}
		}
		f = fts_read(fts);
	}

	fts_close(fts);

	for (int i = 0; i < days; i++)
	{
		if (strlen(dates[i]) > 0)
			printf("%s\n", dates[i]);		
	}

	free(dates);



	return 0;
}


int dayCount(char *startDate, char *endDate)
{
	int startDay = atoi(startDate+6);
	startDate[6] = 0;
	int startMonth = atoi(startDate+4);
	startDate[4] = 0;
	int startYear = atoi(startDate);

	int endDay = atoi(endDate+6);
	endDate[6] = 0;
	int endMonth = atoi(endDate+4);
	endDate[4] = 0;
	int endYear = atoi(endDate);

	struct tm d = {0};
	d.tm_year = startYear - 1900;
	d.tm_mon = startMonth - 1;
	d.tm_mday = startDay;
	time_t seconds = timegm(&d);

	struct tm e = {0};
	e.tm_year = endYear - 1900;
	e.tm_mon = endMonth - 1;
	e.tm_mday = endDay;
	e.tm_hour = 23;
	e.tm_min = 59;
	e.tm_sec = 59;

	time_t end = timegm(&e);

	int count = 0;
	while (seconds < end)
	{
		d.tm_mday = d.tm_mday + 1;
		seconds = timegm(&d);
		count++;
	}

	return count;
		
}

int initDates(char **dates, char *startDate, char *endDate)
{
	int start = atoi(startDate);

	int startDay = atoi(startDate+6);
	startDate[6] = 0;
	int startMonth = atoi(startDate+4);
	startDate[4] = 0;
	int startYear = atoi(startDate);

	int endDay = atoi(endDate+6);
	endDate[6] = 0;
	int endMonth = atoi(endDate+4);
	endDate[4] = 0;
	int endYear = atoi(endDate);

	struct tm d = {0};
	d.tm_year = startYear - 1900;
	d.tm_mon = startMonth - 1;
	d.tm_mday = startDay;
	time_t seconds = timegm(&d);

	struct tm e = {0};
	e.tm_year = endYear - 1900;
	e.tm_mon = endMonth - 1;
	e.tm_mday = endDay;
	e.tm_hour = 23;
	e.tm_min = 59;
	e.tm_sec = 59;

	time_t end = timegm(&e);

	int count = 0;

	int index = 0;


	char date[255] = {0};
	while (seconds < end)
	{
		sprintf(date, "%04d%02d%02d", d.tm_year + 1900, d.tm_mon+1, d.tm_mday);
		// printf("Start: %d\n", start);
		index = atoi(date) - start;
		// printf("%s %d\n", date, atoi(date));
		dates[index] = malloc(9);
		sprintf(dates[index], "%s", date);

		d.tm_mday = d.tm_mday + 1;
		seconds = timegm(&d);
		count++;

	}

	return count;
		
}