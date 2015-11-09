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

#define MAX_CONNECTIONS 10
#define DEFAULT_PORT "36115" // This can change and should be in our protocol
#define T_TYPE 'T'
#define F_TYPE 'F'
#define E_TYPE 'E'
#define D_TYPE 'D'

#define BUFSIZE 4

typedef struct fileInfo {
  char filename[64];
  int fileLen;
  int padID;
} clientInfo;

void printUsage(char* name) {
  fprintf(stdout, "usage: %s [-h] [-p <port number>]\n", name);
  fprintf(stdout, "       h: displays file usage information\n");
  fprintf(stdout, "       p: takes an argument <port number> that will specify"
	  "the port number to be used. Default port number is 36115\n");
}

int getSocket(char* port) {
  struct addrinfo *res, *i;
  struct addrinfo hints;

  int num; /* variable to check the return value of getaddrinfo */
  int sd; /* variable for the socket descriptor */

  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_PASSIVE | AI_V4MAPPED;
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  num = getaddrinfo(NULL, port, &hints, &res);
  if (num != 0) {
    fprintf(stderr, "Error: %s\n", gai_strerror(num));
    exit(1);
  }

  for (i = res; i = i->ai_next) {
    sd = socket(i->ai_family, i->ai_socktype, i->ai_protocol);

    if (sd == -1) {
      fprintf(stderr, "Error creating socket\n");
      exit(1);
    }

    /* bind port to socket */
    num = bind(sd, i->ai_addr, i->ai_addrlen);
    if (num == -1) {
      fprintf(stderr, "Error binding port to socket\n");
      close(sd);
      exit(1);
    }

    /* listen for incoming connections */
    num = listen(sd, MAX_CONNECTIONS);
    if (num == -1) {
      fprintf(stderr, "Error listening for incomign connections\n");
      close(sd);
      exit(1);
    }

    break;
  }
  freeaddrinfo(res);
  return sd;
}

int getPadOffset(int padID) {
  /*takes padID as input, returns 0 if successful,
    otherwise returns the numbers we have for error
    codes in the protocol */
}

int initConnection(int cd, clientInfo *info) {
  //takes client descriptor and struct clientInfo as parameters
  uint8_t buf[BUFSIZE];
  uint8_t msg[BUFSIZE];
  clientInfo info;
  
  if (read(cd, buf, BUFSIZE) == -1) {
    fprintf(stderr, "Error in read\n");
    exit(1);
  }

  if (buf[0] == T_TYPE) {
    info->filename = buf[1];
    info->fileLen = buf[2];
    info->padID = buf[3];
  }
  
  msg[0] = T_TYPE;
  if ((msg[1] = getPadOffset(info->padID)) != 0) {
    msg[0] = E_TYPE;
  }
  
  if (write(cd, msg, BUFSIZE) == -1) {
    fprintf(stderr, "Error in write\n");
    exit(1);
  } 
  return 0;
}

int acceptCon(int socket) {
  /* Handles client requests 
     will be updated to support Port Knocking */

  struct sockaddr_storage clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  int cd;

  cd = accept(socket, (struct sockaddr *) &clientAddr, &clientAddrLen);
  if (cd == -1) {
    fprintf(stderr, "Error accepting connections\n");
    exit(1);
  }
  return cd;
}

void *worker(cd) {
  clientInfo *info = malloc(sieof(info));
  if (info == NULL) {
    fprintf(stderr, "Memory allocation failure\n");
    exit(1);
  }

  initConnection(cd, &info);
  //Function that actually transfers the file
  free(info);
  return NULL;
}

int main(int argc, char* argv[]) {
  int opt, sd, cd;
  char *port = DEFAULT_PORT;
  pthread_t tid; 

  
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
  //threading goes here
  sd = getSocket(port); //This should be in netCode.h
  cd = acceptCon(sd);
  pthread_create(&tid, NULL, worker, &cd);
  pthread_join(tid, NULL);
  /*the above 4 lines should continue forever. While(1) or While(1) and Select? */
  return 0;
}
  
