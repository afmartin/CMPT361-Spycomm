/*
#################################################################################
CMPT 361 - Assignment 3                                                         
Group 4: Nick, John, Alex, Kevin
November 6th, 2015
Filename: log.h 
Description: Functions for logging 
#################################################################################
*/

#include <stdbool.h> // necessary for function prototypes
#include <stdio.h> // sometimes when this is included before stdio, it complains about FILE

/**
 * initLog
 *
 * Opens the log file the program will write to based on the
 * specified filename.
 *
 * Be advised: the file handle will be open during the entire program execution.
 * It will only be closed with closeProgram is called from this file.
 *1
 * Args:
 * char * f - the filename
 */
void initLog(char * f);

/**
 * closeProgram
 *
 * A custom program exit function that will end ncurses if applicable,
 * close the log and called exit with a proper return code.
 *
 * Warning: No where in the program should call exit when the log
 * is initialized, this function should be called instead.
 *
 * Args:
 * bool crashed - is the program needing to terminate because of an error?
 * bool ncurses - is an ncurses screen active?
 */
void closeProgram(bool crashed, bool ncurses);

/**
 * getLog()
 *
 * Function to retrieve the log file pointer.  
 *
 * Warning: make sure instantiateLog is called first.
 *
 * Returns FILE * to log file.
 */
FILE * getLog();


