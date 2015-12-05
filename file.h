/*
#############################################################
CMPT 361 - Assignment 3
Group 4: Nick, John, Alex, Kevin
November 11th, 2015
Filename: file.h
Description:
#############################################################
*/

#ifndef _FILE_H
#define _FILE_H
#define MAX_PACKET_LEN 512

long long int getFileSizeFromFilename(char * filename);
/* This function takes a filename as parameters and returns
 * the size of the file in bytes. */ 

long long int getFileSize(int fd);
/* The function takes a file descriptor as parameters and
   returns the size of the file in bytes. */

uint8_t ** getFileArray(FILE* file, unsigned long fileSize);
/* Takes a file pointer, the size of the file in bytes, and a
   pointer to a uint8_t array as parameters. Puts the contents
   of the file into the array. Returns 0 on success and -1
   on failure. */

int writeToFile(char* filename, uint8_t *byteArray, int packetLen);
/* Takes string of the filename to be created, a pointer to a
   uint8_t array containing bytes to be written, and an integer
   representing the amount of bytes to be written. Will write
   to the file and return 0 on success and -1 on failure. */

void printByteArray(uint8_t * byteArray);
/* Takes uint8_t array containing bytes to be printed, and the
   total size of the array. */

void getCurrentTime(char* timeString);
/*returns a string containing the current time */
#endif /* end _FILE_H if */
