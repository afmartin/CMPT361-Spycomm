/*
#############################################################
CMPT 361 - Assignment 3
Group 4: Nick, John, Alex, Kevin
November 11th, 2015
Filename: file.h
Description: Functions for reading and writing files.
#############################################################
*/

#ifndef _FILE_H
#define _FILE_H
#define MAX_PACKET_LEN 512

/**
 * getFileSizeFromFilename
 * 
 * The function gets the file size and returns it.
 * 
 * Args:
 * char* filename - the name of the file
 * in question.
 *
 * Returns:
 * a long long integer which represents the
 * size of the file
 */ 

long long int getFileSizeFromFilename(char * filename);

/**
 * getFileSize                 
 * The function gets the file size and returns it.
 *
 * Args:
 * int fd - the file descriptor of the file in
 * question.
 *
 * Returns:
 * a long long integer which represents the
 * size of the file.
 */

long long int getFileSize(int fd);

/**
 * getFileArray
 * 
 * Reads from a file puts it's contents into an array.
 * 
 * Args:
 * FILE* file - pointer to the file in question
 * unsigned long fileSize - size of the file to be read.
 * 
 * Returns:
 * returns an array of arrays.
 */

uint8_t ** getFileArray(FILE* file, unsigned long fileSize);

/**
 * writeToFile
 * 
 * Creates and writes to a file if the name does not exist. Otherwise
 * appends to the file.
 *
 * NOTE: This function isn't currently in use. It was used early on
 * in the problem and exists for testing purposes.
 *
 * Args:
 * char* filename - the name of the file in question.
 * uint8_t *byteArray - an array of bytes to be written
 * int packetLen - the size of the packet denoting the 
 * size of the segment to be written.
 *
 * Returns:
 * returns 0 on success, and -1 on failure
 */

int writeToFile(char* filename, uint8_t *byteArray, int packetLen);

/**
 * printByteArray
 *
 * Simply prints out the contents of a uint8_t array to stdout.
 *
 * NOTE: This function exists for testing.
 *
 * Args:
 * uint8_t * byteArray - the array to be printed out
 */

void printByteArray(uint8_t * byteArray);

/**
 * getCurrentTime
 *
 * gets the current time and sets it to a string.
 *
 * Args:
 * char* timeString - the string that will contain the current time.
 */

void getCurrentTime(char* timeString);
#endif /* end _FILE_H if */
