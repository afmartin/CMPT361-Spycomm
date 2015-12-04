/*j############################################################
CMPT 361 - Assignment 3
Group 4: Nick, John, Alex, Kevin
November 9th, 2015
Filename: file.c
Description: Contains various functions for writing and recieving
files that are used in server.c and client.c 
#############################################################
*/

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#include "file.h"
#include "log.h"

long long int getFileSize(int fd) {
  //initialize stat structure from sys/stat.h
  struct stat buf;
  size_t size;
  
  //call fstat on file descriptor, pass buf struct
  fstat(fd, &buf);
  //get file size from st_size member
  size = buf.st_size; 
  
  return size;
}

uint8_t ** getFileArray(FILE* file, unsigned long fileSize) {
  //calculate number of packets and set to a variable
  int amountOfPackets = fileSize / MAX_PACKET_LEN ;
  //intialize array of array
  uint8_t ** byteArray;

  //allocate memory for number of indexes, and for each individual index
  byteArray = malloc(sizeof(uint8_t *) * amountOfPackets);
  for (int i = 0; i < amountOfPackets+1; i++)
    byteArray[i] = malloc(MAX_PACKET_LEN);

  //call fread on file, read MAX_PACKET_LEN bytes of data into each index of array
  for(int i = 0; i < amountOfPackets+1; i++){
    size_t read = fread(byteArray[i], 1, MAX_PACKET_LEN, file);
    if (read == 0) {
      fprintf(getLog(), "ERROR: Error reading from file with descriptor:'%d'\n", fileno(file));
      return NULL;
    }
  }

  return byteArray;
}

int writeToFile(char* filename, uint8_t *byteArray, int packetLen) {
  uint8_t uint8Chr;
  FILE *fp;

  /* referenced from stackoverflow.com/questions/13002367/write-a-file
     -byte-by-byte-in-c-using-fwrite */

  //open file to write
  fp = fopen(filename, "a");
  if (fp == NULL) {
    fprintf(getLog(), "ERROR: Error opening file '%s'\n", filename);
    return 0;
  }
  //iterate through array, and write each character to the file
  for (int i = 0; i < packetLen; i++) { 
    uint8Chr = byteArray[i];
    /* if (uint8Chr == 0)  */
    /*   break; */
    unsigned char c = (unsigned char) uint8Chr; 
    fwrite(&c, 1, sizeof(c), fp); //write character to file
  }
  
  fclose(fp);
  return 1;
}

void printByteArray(uint8_t * byteArray) {
  int i;
  
  //iterate through array, printing each character to stdout
  for (i = 0; i < MAX_PACKET_LEN; i++)
    fprintf(stdout, "%c ", byteArray[i]);
}

void getCurrentTime(char* timeString) {
  /* referenced from stackoverflow.com/questions/5141960/get-the-current-time-in-c */

  time_t currTime;
  struct tm* timeInfo;

  //set time string
  time (&currTime);
  timeInfo= localtime(&currTime);
  strftime(timeString, 100, "%F:%T", timeInfo);
}
	       
