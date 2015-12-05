/*
#################################################################################
CMPT 361 - Assignment 3                                                         
Group 4: Nick, John, Alex, Kevin
November 6th, 2015
Filename: log.c 
Description: Functions for logging 
#################################################################################
*/

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>

#include "log.h"

static FILE * log = NULL; // initializes to null in case someone calls getLog prematurely.
static char * filename;

void initLog(char * f) {
	filename = f;
	log = fopen(filename, "a");
	if (log == NULL) {
		fprintf(stderr, "ERROR: Log file could not be opened, will not log.");
		perror("fopen");
	} else {
		// Ref: http://stackoverflow.com/questions/8622735/fopen-fprintf-dosnt-write-to-file-only-after-closing-it
		setvbuf(log, NULL, _IOLBF, 0); // Have it save to disk after every line is written
		time_t t;
		time(&t);
		// Note: ctime has a new line character at the end
		fprintf(log, "LOG: Started at %s", ctime(&t));
	}
}

void closeProgram(bool crashed, bool ncurses) {
	if (ncurses) {
		endwin();
	}
	fclose(log);

	if (crashed) {
		fprintf(stdout, "Program has terminated prematurely.  View %s for details.\n", filename);
	}

	// if crashed return 1, if not return 0
	exit(crashed ? 1 : 0);
}

FILE * getLog() {
	if (log == NULL) {
		fprintf(stderr, "ERROR: Log is not initialized yet\n");
	}
	return log;
}

