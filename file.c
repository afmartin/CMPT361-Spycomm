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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int getFileSize(int fd, char* filename) {
  struct stat buf;
  int size;
  
  fstat(fd, &buf);
  size = buf.st_size;

  printf("Size of %s is %d\n", filename, size);
  return 0;
}


int main(void) {
  FILE *fp;
  int fd;
  char filename[] = "server.c";
  
  fp = fopen(filename, "r");
  fd = fileno(fp);
  getFileSize(fd, filename);
  return 0;
}
