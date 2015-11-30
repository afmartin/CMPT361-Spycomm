/*
#############################################################
CMPT 361 - Assignment 3
Group 4: Nick, John, Alex, Kevin
November 9th, 2015
Filename: file.c
Description:
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

//#define DONES printf("done");


long long int getFileSize(int fd) {
  struct stat buf; //init stat structure from sys/stat.h
  size_t size;
  
  fstat(fd, &buf); //call fstat on file descriptor, pass buf struct
  size = buf.st_size; //get file size from structure

  return size;
}

uint8_t ** getFileArray(FILE* file, unsigned long fileSize) {
  
  int amountOfPackets = fileSize / MAX_PACKET_LEN ;
  uint8_t ** byteArray;
  byteArray = malloc(sizeof(uint8_t *) * amountOfPackets);
  for (int i = 0; i < amountOfPackets+1; i++)
    byteArray[i] = malloc(MAX_PACKET_LEN);

  //call fread on file
  for(int i = 0; i < amountOfPackets+1; i++){
    size_t read = fread(byteArray[i], 1, MAX_PACKET_LEN, file);
    if (read == 0) {
      fprintf(stderr, "Error reading file\n");
      return NULL;
    }
  }

  //byteArray[newLen+1] = '\0'; //add escape character
  return byteArray;
}

int writeToFile(char* filename, uint8_t *byteArray, int packetLen) {
  int k;
  uint8_t uint8Chr;
  FILE *fp;

  //  k = MAX_PACKET_LEN; //calculate how many indexes in array
  //DONES;
  /* referenced from stackoverflow.com/questions/13002367/write-a-file
     -byte-by-byte-in-c-using-fwrite */
  //printf("called\n");
  fp = fopen(filename, "ab+"); //open file to write
  if (fp == NULL) {
    fprintf(stderr, "Error opening file '%s'\n", filename);
    return -1;
  }
  //DONES;
  for (int i = 0; i < packetLen; i++) { //iterate through array
    //for (int ii = 0; ii < MAX_PACKET_LEN; ii++){
    uint8Chr = byteArray[i];
    /* if (uint8Chr == 0)  */
    /*   break; */
    unsigned char c = (unsigned char) uint8Chr; 
    fwrite(&c, 1, sizeof(c), fp); //write character to file
  }
  
  fclose(fp);
  //printf("Closed the file!\n");
  return 1;
}

void printByteArray(uint8_t * byteArray) {

  //for (int i = 0; i < k+1; i++)
  for (int i = 0; i < MAX_PACKET_LEN; i++)
    fprintf(stdout, "%c ", byteArray[i]);
}

void getCurrentTime(char* timeString) {
  /* referenced from stackoverflow.com/questions/5141960/get-the-current-time-in-c */

  time_t currTime;
  struct tm* timeInfo;

  time (&currTime);
  timeInfo= localtime(&currTime);
  strftime(timeString, 100, "%F:%T", timeInfo);
  //timeString = asctime(timeInfo);
  //$$  printf("Current local time and date: %s\n", timeString);

}
	       
//main function used to test above functions

/*int main(void) {
  FILE *fp, *fp2;
  int fd, size, i, k;
  uint8_t *array;
  char filename[] = "server.c";
  char filename2[] = "asdf.c";

  
  fp = fopen(filename, "r");
  fd = fileno(fp);
  size = getFileSize(fd);
  k = (size / sizeof(uint8_t));
  array = (uint8_t*) malloc((size+1)*sizeof(uint8_t));
  getFileArray(fp, size, array);
  close(fd);
  printByteArray(array, size);
  writeToFile(filename2, array, size);
  free(array);
  return 0;
  }*/ 

