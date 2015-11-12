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
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>

int getFileSize(int fd) {
  struct stat buf;
  int size;
  
  fstat(fd, &buf);
  size = buf.st_size;

  return size;
}

int getFileArray(FILE* file, int fileSize,  uint8_t *byteArray) {
  size_t newLen = fread(byteArray, 1, fileSize, file);
  if (newLen == 0) {
    fprintf(stderr, "Error reading file\n");
    return -1;
  } else {
    byteArray[newLen++] = '\0';
  }
  return 0;
}

int writeToFile(char* filename, uint8_t *byteArray, int fileSize) {
  int i, k;
  uint8_t chr;
  FILE *fp;

  k = (fileSize / sizeof(uint8_t));

  /* referenced from stackoverflow.com/questions/13002367/write-a-file
     -byte-by-byte-in-c-using-fwrite */
  fp = fopen(filename, "w+");
  if (fp == NULL) {
    fprintf(stderr, "Error opening file '%s'\n", filename);
    return -1;
  }

  for (i = 0; i < k; i++) {
    chr = byteArray[i];
    if (chr == EOF) {
      break;
    }
    char c = (char) chr;
    fwrite(&c, 1, sizeof(c), fp);
  }

  close(fp);
  return 0;
}

void printByteArray(uint8_t* byteArray, int fileSize) {
  int i, k;
  k = (fileSize / sizeof(uint8_t));  

  for (i = 0; i < k; i++) 
    fprintf(stdout, "%x ", byteArray[i]);
}
	       
/*main function used to test above functions

int main(void) {
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
  } */

