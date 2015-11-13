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
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "server.h"
#include "file.h"

#define DONE printf("done\n")
#define MAX_CONNECTIONS 10
#define DEFAULT_PORT "36115" // This can change and should be in our protocol
#define T_TYPE 'T'
#define F_TYPE 'F'
#define E_TYPE 'E'
#define D_TYPE 'D'

#define MAX_THREAD 10
#define TRUE 1
#define FALSE 0
#define MAXLEN 64
#define MAX_FILE_NAME 64
#define MAX_MD5LEN 16
#define MAX_FILE_LENGTH_AS_STRING 10

/* typedef struct _threadArgs{ */
  
/*   int sockfd; */
/*   fileInfo info; */
/* } threadArgs; */

typedef struct _fileInfo {
  char filename[MAX_FILE_NAME];
  int fileLen;
  char padID[MAX_MD5LEN + 1];
} fileInfo;

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

  for (i = res; i;  i = i->ai_next) {
    sd = socket(i->ai_family, i->ai_socktype, i->ai_protocol);

    if (sd == -1) {
      fprintf(stderr, "Error creating socket\n");
      continue;
    }

    /* bind port to socket */
    num = bind(sd, i->ai_addr, i->ai_addrlen);
    if (num == -1) {
      fprintf(stderr, "Error binding port to socket\n");
      continue;
    }

    /* listen for incoming connections */
    num = listen(sd, MAX_CONNECTIONS);
    if (num == -1) {
      fprintf(stderr, "Error listening for incomign connections\n");
      continue;
    }

    break;
  }
  if (num == -1 || sd == -1){
    fprintf(stderr, "Could not form a socket\n");
    freeaddrinfo(res);
    exit (1);
  }
  freeaddrinfo(res);
  return sd;
}

int getPadOffset(int padID) {

  return 0;
  /*takes padID as input, returns 0 if successful,
    otherwise returns the numbers we have for error
    codes in the protocol */
}

// function to send a string over a socket descriptor
int send_string(int sd, uint8_t * buf){
  
  size_t queued = MAXLEN;
  ssize_t sent;
  while (queued > 0){
    sent = send(sd, buf, queued, 0);
    if (sent == -1)
      return FALSE;
    queued -= sent;
    buf += sent;
  }
  return TRUE;
}

int initFileTransfer(int cd, fileInfo *info) {
  
  //takes client descriptor and struct clientInfo as parameters
  uint8_t buf[MAXLEN];
  uint8_t * ptr = buf+1;
  uint8_t msg[MAXLEN];
  
  char temp[MAXLEN];
  
  if (recv(cd, buf, MAXLEN, 0) == -1) {
    fprintf(stderr, "Error in read\n");
    exit(1);
  }

  if (buf[0] == T_TYPE) {
    int position = 0;
    
    // copy filename into struct 
    while (*ptr != '\0') {
		if (position == sizeof(info->filename) - 1){
			break;
		}
		temp[position++] = *ptr++;
	}
    temp[position] = '\0'; 
	
	//Copy the string into the struct
	strcpy(info->filename, temp);
	
	//Skip over the null terminator and reset the position
    ptr++;
    position = 0;
	
    // copy fileLen into struct
    while (*ptr != '\0'){
		if (position == MAX_FILE_LENGTH_AS_STRING){
			fprintf(stderr, "File size requested too large!\n");
			exit(1);
		}
		temp[position++] = *ptr++;
    }
	temp[position] = '\0';
	
	//convert the string to an int
	info->fileLen = atoi(temp);
	
    ptr++;
    position = 0;
	
    //copy the PadID into the struct
    while (*ptr != '\0'){
		if (position == sizeof(info->padID) - 1){
			break;
		}
		temp[position++] = *ptr++;
	}
	temp[position] = '\0';

	//Copy the string into the struct
	strcpy(info->padID, temp);
	
    printf("%s -- %d -- %s", info->filename, info->fileLen, info->padID);
  }
  else return FALSE;
  
  msg[0] = T_TYPE;
  // save for later -->
  /*if ((msg[1] = getPadOffset(info->padID)) != 0) {
    msg[0] = E_TYPE;
    }*/  
  msg[1] = '\0';
  
  
  if (!send_string(cd, msg)) {
    fprintf(stderr, "Error sending response\n");
  } 
  return TRUE;
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

void* worker(void * arg) { //this is the function that threads will call
  
  int done = 0;
  int * cd = (int *) arg;
  uint8_t packet[MAXLEN];
  size_t len = MAXLEN;
  ssize_t received;
  fileInfo *info = malloc(sizeof(info));
  if (info == NULL) {
    fprintf(stderr, "Memory allocation failure\n");
    exit(1);
  }

  if (initFileTransfer(*cd, info)){
    
    uint8_t * fileContents; //+1 to allow for null term
    printf("%d\n", (*info).fileLen);
    fileContents = malloc(sizeof(uint8_t) * (*info).fileLen);
    uint8_t * ptr = fileContents; // set pointer to start of fileContents
    while(!done){
      received = recv(*cd, packet, len, 0);
      if(packet[0] == 'D'){ // check if client is finished sending
	done = 1;
	break;
      }
      if (packet[0] == 'F'){
	writeToFile("HELLOWORLD", packet+1, 11);
	/*for (int i = 1; i < received; i++)
	 
	  //*ptr++ = packet[i]; // set 
	  }*/
      }
    }
    if (done){} // output to file
  }
      
  //Function that actually transfers the file
  free(info);
  return NULL;
}

int main(int argc, char* argv[]) {
  
  int opt, sd, cd;
  char *port = DEFAULT_PORT;
  pthread_t tid; 

  
  while ((opt = getopt(argc, argv, "hp:")) != -1) {

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
  
  while (TRUE){
    //threading goes here
    sd = getSocket(port); //This should be in netCode.h
    cd = acceptCon(sd);    
    pthread_create(&tid, NULL, worker, &cd);
    pthread_join(tid, NULL);
  }
  return 0;
}
  
