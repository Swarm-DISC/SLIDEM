/*

    SLIDEM Processor: util/slidemParallel/main.c

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
#include <unistd.h>

#include <pthread.h>

#include <time.h>
#include <curses.h>


#define SOFTWARE_VERSION "1.0"

#define THREAD_MANAGER_WAIT 100000 // uSeconds

#define MAX_THREADS 38


enum STATUS 
{
	STATUS_OK = 0,
	STATUS_PERMISSION,
	STATUS_MEM
};

typedef struct CommandArgs
{
	bool threadRunning;
	int returnValue;
	char *satLetter;
	char *lpDir;
	char *modDir;
	char *magDir;
	char *exportDir; 
	char *date;
} CommandArgs;

int dayCount(char *startDate, char *endDate);
void incrementDate(char *date);

static int rows = 0;
static int cols = 0;

#define SLIDEM_LABEL 0, 0
#define SAT_ORIGIN 1,1
#define START_DATE_ORIGIN 2,1
#define END_DATE_ORIGIN 3,1
#define START_TIME_ORIGIN 5,1
#define PROCESSING_TIME_ORIGIN 6, 1
#define PROCESSING_STATUS_ORIGIN 8,3
#define KEYBOARD_ORIGIN 10,2

void initScreen(void);

void *runThread(void *a);

int main(int argc, char *argv[])
{

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--about") == 0)
        {
            fprintf(stdout, "slidemParallel version %s.\n", SOFTWARE_VERSION);
            fprintf(stdout, "Copyright (C) 2022  Johnathan K Burchill\n");
            fprintf(stdout, "This program comes with ABSOLUTELY NO WARRANTY.\n");
            fprintf(stdout, "This is free software, and you are welcome to redistribute it\n");
            fprintf(stdout, "under the terms of the GNU General Public License.\n");
            exit(0);
        }
    }

	if (argc !=  9)
	{
		printf("usage:\t%s satellite lpDirectory modDirectory magDirectory exportDirectory startyyyymmdd endyyyymmdd nthreads\n\t\tparallel processes Swarm LP data to generate SLIDEM product for specified satellite and date.\n", argv[0]);
		printf("\t%s --about\n\t\tprints copyright and license information.\n", argv[0]);
		exit(0);
	}


	initScreen();
	clear();

	char *satelliteLetter = argv[1];
	char *lpDir = argv[2];
	char *modDir = argv[3];
	char *magDir = argv[4];
	char *exportDir = argv[5];
	char *startDate = argv[6];
	char *endDate = argv[7];
	int nThreads = atoi(argv[8]);
	if (nThreads > MAX_THREADS)
	{
		nThreads = MAX_THREADS;
	}

	char *date = strdup(startDate);
	char *d1 = strdup(startDate);
	char *d2 = strdup(endDate);
	int days = dayCount(d1, d2);
	free(d1);
	free(d2);

	CommandArgs *commandArgs = calloc(nThreads, sizeof(CommandArgs));
	if (commandArgs == NULL)
	{
		endwin();
		printf("Could not calloc memory for thread arguments.\n");
		exit(EXIT_FAILURE);
	}

	// Loop and wait for threads

	pthread_t threadIds[MAX_THREADS] = {0};

	pthread_attr_t attr;

	int status = pthread_attr_init(&attr);
	if (status)
	{
		endwin();		
		printf("Could not init pthread attributes.\n");
		exit(EXIT_FAILURE);
	}
	int completed = 0;
	int queued = 0;

	time_t startTime = time(NULL);
	time_t currentTime = 0;
	struct tm *now = localtime(&startTime);
	long t = 0;
	int seconds = 0;
	int minutes = 0;
	int hours = 0;
	mvprintw(SLIDEM_LABEL, "SLIDEM processor");
	mvprintw(SAT_ORIGIN, "Swarm %s (%d threads)", satelliteLetter, nThreads);
	mvprintw(START_DATE_ORIGIN, "From: %s\n", startDate);
	mvprintw(END_DATE_ORIGIN, "  To: %s\n", endDate);
	mvprintw(START_TIME_ORIGIN, "Started: %4d%02d%02d %02d:%02d:%02d", now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
	mvprintw(PROCESSING_TIME_ORIGIN, "Total time: %02d:%02d:%02d", 0, 0, 0);
	mvprintw(PROCESSING_STATUS_ORIGIN, "%d/%d processed (%4.1f%%)", completed, days, (float)completed / (float)days * 100.0);

	mvprintw(KEYBOARD_ORIGIN, "[q] - quit");
	refresh();

	int keyboard = 0;

	while (completed < days)
	{
		for (int i = 0; i < nThreads && completed < days; i++)
		{
			if (!commandArgs[i].threadRunning)
			{
				// Get return value from completed thread if applicable
				if (threadIds[i] > 0)
				{
					status = pthread_join(threadIds[i], NULL);
					if (status == 0)
					{
						completed++;
						commandArgs[i].threadRunning = false;
						mvprintw(PROCESSING_STATUS_ORIGIN, "%d/%d processed (%4.1f%%)", completed, days, (float)completed / (float)days * 100.0);
						clrtoeol();
						threadIds[i] = 0;
					}
				}
				// start a new thread
				if (queued < days)
				{
					commandArgs[i].threadRunning = true;
					commandArgs[i].satLetter = satelliteLetter;
					commandArgs[i].lpDir = lpDir;
					commandArgs[i].modDir = modDir;
					commandArgs[i].magDir = magDir;
					commandArgs[i].exportDir = exportDir;
					commandArgs[i].date = strdup(date);
					// printf("preparing to process %s.\n", commandArgs[i].date);
					commandArgs[i].returnValue = 0;
					pthread_create(&threadIds[i], &attr, &runThread, (void*) &commandArgs[i]);
					incrementDate(date);
					queued++;
				}
			}
		}
		currentTime = time(NULL);
		t = (long)currentTime - (long)startTime;
		hours = t / 3600;
		minutes = (t - 3600*hours) / 60;
		seconds = t - 3600*hours - 60 * minutes;
		mvprintw(PROCESSING_TIME_ORIGIN, "Total time: %02d:%02d:%02d", hours, minutes, seconds);

		keyboard = getch();
		if (keyboard != ERR)
		{
			switch (keyboard)
			{
				case 'q':
					goto exit;
					break;
				default:
					break;
			}
		}

		refresh();
		usleep(THREAD_MANAGER_WAIT);
	}

exit:
	status = pthread_attr_destroy(&attr);
	free(commandArgs);
	endwin();
	printf("Days processed: %d / %d\n", completed, days);

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

void incrementDate(char *date)
{
	int start = atoi(date);

	int startDay = atoi(date+6);
	date[6] = 0;
	int startMonth = atoi(date+4);
	date[4] = 0;
	int startYear = atoi(date);

	struct tm d = {0};
	d.tm_year = startYear - 1900;
	d.tm_mon = startMonth - 1;
	d.tm_mday = startDay + 1;
	timegm(&d);
	sprintf(date, "%04d%02d%02d", d.tm_year + 1900, d.tm_mon+1, d.tm_mday);

	return;
		
}

void initScreen(void)
{
	initscr();
    getmaxyx(stdscr, rows, cols);
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);
}

void *runThread(void *a)
{
	CommandArgs* args = (CommandArgs *)a;

	// exec slidem command 
	int status = 0;
	char command[5*FILENAME_MAX+256];
	sprintf(command, "slidem %s %s %s %s %s %s > %s/%s%s.log 2>&1", args->satLetter, args->date, args->lpDir, args->modDir, args->magDir, args->exportDir, args->exportDir, args->satLetter, args->date);
	status = system(command);
	if (WIFEXITED(status) && (WEXITSTATUS(status) == 0))
	{
		// Use mvprintw() from curses to have a scrolling log?
	}
	else
	{
		// mvprintw?
	}
	args->threadRunning = false;
	args->returnValue = status;
	pthread_exit(NULL);
}
