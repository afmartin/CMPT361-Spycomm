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

size_t getFileSize(int fd);
/* The function takes a file descriptor as parameters and
   returns the size of the file in bytes. */

uint8_t ** getFileArray(FILE* file, int fileSize);
/* Takes a file pointer, the size of the file in bytes, and a
   pointer to a uint8_t array as parameters. Puts the contents
   of the file into the array. Returns 0 on success and -1
   on failure. */

int writeToFile(char* filename, uint8_t **byteArray, int fileSize);
/* Takes string of the filename to be created, a pointer to a
   uint8_t array containing bytes to be written, and an integer
   representing the amount of bytes to be written. Will write
   to the file and return 0 on success and -1 on failure. */

void printByteArray(uint8_t ** byteArray, int fileSize);
/* Takes uint8_t array containing bytes to be printed, and the
   total size of the array. */

#endif /* end _FILE_H if */
