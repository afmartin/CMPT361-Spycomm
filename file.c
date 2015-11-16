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
#include "file.h"

//#define DONES printf("done");


size_t getFileSize(int fd) {
  struct stat buf; //init stat structure from sys/stat.h
  size_t size;
  
  fstat(fd, &buf); //call fstat on file descriptor, pass buf struct
  size = buf.st_size; //get file size from structure

  return size;
}

uint8_t ** getFileArray(FILE* file, int fileSize) {
  
  int amountOfPackets = fileSize / MAX_PACKET_LEN;
  uint8_t ** byteArray;
  byteArray = (uint8_t **)malloc
    ((sizeof(uint8_t) * MAX_PACKET_LEN) * amountOfPackets);

  //call fread on file
  for(int i = 0; i < amountOfPackets; i++){
    size_t read = fread(byteArray[i], 1, MAX_PACKET_LEN, file);
    if (read == 0) {
      fprintf(stderr, "Error reading file\n");
      return NULL;
    }
  }

  //byteArray[newLen+1] = '\0'; //add escape character
  return 0;
}

int writeToFile(char* filename, uint8_t *byteArray, int fileSize) {
  int i, k;
  uint8_t chr;
  FILE *fp;

  k = (fileSize / sizeof(uint8_t)); //calculate how many indexes in array
  //DONES;
  /* referenced from stackoverflow.com/questions/13002367/write-a-file
     -byte-by-byte-in-c-using-fwrite */
  fp = fopen(filename, "wb+"); //open file to write
  if (fp == NULL) {
    fprintf(stderr, "Error opening file '%s'\n", filename);
    return -1;
  }
  //DONES;
  for (i = 0; i < k; i++) { //iterate through array
    chr = byteArray[i];
    if (chr == EOF) {
      break;
    }
    char c = (char) chr; 
    fwrite(&c, 1, sizeof(c), fp); //write character to file
  }
  //DONES;
  fclose(fp);
  printf("Closed the file!\n");
  return 1;
}

void printByteArray(uint8_t ** byteArray, int fileSize) {
  int k;
  k = fileSize / MAX_PACKET_LEN;  

  for (int i = 0; i < k; i++)
    for (int ii = 0; ii < MAX_PACKET_LEN; ii++)
      fprintf(stdout, "%x ", byteArray[i][ii]);
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

