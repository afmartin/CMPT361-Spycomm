/*
#################################################################################
CMPT 361 - Assignment 3                                                         
Group 4: Nick, John, Alex, Kevin
November 6th, 2015
Filename: server.c
Description:
#################################################################################
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <getopt.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "server.h"

#define DEFAULT_PORT "36115" // This can change and should be in our protocol


void printUsage(char* name) {
  fprintf(stdout, "usage: %s [-h] [-p <port number>]\n", name);
  fprintf(stdout, "       h: displays file usage information\n");
  fprintf(stdout, "       p: takes an argument <port number> that will specify"
	  "the port number to be used. Default port number is 36115\n");
}

int acceptCon(int socket) {
  /* Handles client requests */

  struct sockaddr_storage clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  int cd;
  pthread_t tid;

  cd = accept(socket, (struct sockaddr *) &clientAddr, &clientAddrLen);
  if (cd == -1) {
    fprintf(stderr, "Error accepting connections\n");
    exit(1);
  }

  //pthread_create(&tid, NULL, function that handles decrypt->decode, 
  //func parameters);
//pthread_join(tid, NULL);

  return 0;
}

int main(int argc, char* argv[]) {
  int opt, sd;
  char *port = DEFAULT_PORT;
  
  while (opt = getopt(argc, argv, "hp:") != -1) {

    switch(opt) {
    /* Reference used from www.gnu.org/software/libc/manual/html_node/Example-of                                   
       _Getopt.html */
      
    case 'h':
      printUsage(argv[0]);
      exit(0);
      break;
    case 'p':
      port = optarg;
      break;
    default:
      printf("Invalid parameter\n");
    }
  }

  //sd = getSocket(port); This should be in netCode.h
  //acceptCon(sd);
  
  return 0;
}
  
