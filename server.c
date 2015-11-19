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
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include "server.h"
#include "file.h"
#include "netCode.h"
//#include "digest.h"

#define DONE printf("done\n")
#define MAX_CONNECTIONS 10
#define DEFAULT_PORT "36115" // This can change and should be in our protocol

#define MAX_THREAD 5

/* typedef struct _threadArgs{ */
  
/*   int sockfd; */
/*   fileInfo info; */
/* } threadArgs; */

volatile int running = 1;

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
	
    printf("%s -- %d -- %s\n", info->filename, info->fileLen, info->padID);
  }
  else return FALSE;
  
  msg[0] = T_TYPE;
  // save for later -->
  /*if ((msg[1] = getPadOffset(info->padID)) != 0) {
    msg[0] = E_TYPE;
    } 
  msg[1] = '\0';
  
  
  if (!send_string(cd, msg)) {
    fprintf(stderr, "Error sending response\n");
  } */
  return TRUE;
}


void* worker(void * arg) { //this is the function that threads will call
	
	int done = 0;
	int k = 0;
	//uint8_t* temp;
	//char* filePath;
	int * sd = (int*) arg;
	uint8_t packet[MAXLEN + 1];
	memset(packet, 0 , MAXLEN + 1);
	//size_t len = MAXLEN;
	uint8_t ack = 'A';
	
	uint8_t * fileContents; //+1 to allow for null term

	
	printf("value of me is %u\n", (unsigned int) pthread_self()); //check thread id			
	while (TRUE){
		int cd = acceptCon(*sd); //wait for a client to connect
		
		
		while (cd) { // full file transfer loop, allows for multiple filetrans
			fileInfo *info = (fileInfo *)malloc(sizeof(fileInfo)); 
			if (info == NULL) {
				fprintf(stderr, "Memory allocation failure\n");
				exit(1);
			}
			
			
			if (initFileTransfer(cd, info)){
	
				fileContents = malloc(sizeof(uint8_t) * info->fileLen);
				uint8_t * ptr = fileContents; // set pointer to start of fileContents
				while(!done){ // accepting a single file loop
					
					//Determines how many bytes we need to receive
					//Either the Max packet length, or whatever is 
					//remaining at the end of the file
					int get;
					int left = info->fileLen - (ptr - fileContents);
					if (left < MAXLEN + 1){
						get = left;
					}
					else {
						get = MAXLEN;
					}
					
					//Send an acknowledgement so the client knows when it should send data
					sendAll(cd, &ack, sizeof(ack));
					
					//If we are done receiving, get the 'D'
					if (get == 0){
						int didRecv = recv(cd, packet, 1, 0);
						if (didRecv == -1){
						printf("Received Failed!\n");
						pthread_exit(NULL);
						}
					}
					//Otherwise, recv as much as we need
					else {
						int didRecv = recvAll(cd, get + 1, packet);
						if (didRecv == -1){
							perror("");
							printf("Received Failed!\n");
							pthread_exit(NULL);
						}
						if (didRecv != get + 1){
							printf("Bad Coding!\n");
							pthread_exit(NULL);
						}
					}
					
					//printByteArray(packet);
					
					//if we receive the 'D'
					if(packet[0] == (uint8_t) 'D'){
						//DONE;
						sendAll(cd, &ack, sizeof(ack));
						done = 1;
						break;
					}
					//if we receive another packet
					else if (packet[0] == (uint8_t) 'F'){
						//DONE;
						//getMd5Digest(packet+1, info->fileLen, temp);
						//convertMd5ToString(filePath, temp);
						//strcat(folder, filePath); //concat file path with md5 digest
						
						//copy the data from the packet into the fileContents
						//And then increment the pointer
						memcpy(ptr, packet + 1, get);
						ptr += get;
					}
					else {
						printf("Received erroneous data!\n");
						printf("%s\n", packet);
					}
					memset(packet, 0, sizeof(packet));
				}
			} else{
				free(info);
				break;
			}
			//Add a folder prefix
			char folder[100] = "./serverfiles/";
			strcat(folder, (*info).filename);
			
			//Open a file and write the fileContents to it
			FILE * fp = fopen(folder, "wb+");
			k = fwrite(fileContents, 1, info->fileLen + 1, fp);
			fclose(fp);
			
			if (done) {
				close(cd);
				break;
			}
			free(info);
			free(fileContents);
		}
		if (k == -1){ //check return value of write to file
			close(cd);
			printf("Write to File failed!\n");
			pthread_exit(NULL);
		}
		printf("Hello I'm thread %u and I've finished with client %d\n", (unsigned int) pthread_self(), cd );
	}
	//pthread_exit(NULL);
	return NULL;
}
void inputHandler(int s) {
  running = 0;
}
int main(int argc, char* argv[]) {
  int opt, sd, i;
  char *port = DEFAULT_PORT;
  pthread_t tid[MAX_THREAD];

  
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
      printUsage(argv[0]);
      exit(0);
    }
  }
  
  sd = getSocket(port); //This should be in netCode.h
  for (i = 0; i < MAX_THREAD; i++) { //create threads
    pthread_create(&tid[i], NULL, worker, &sd);
  }
  
  signal(SIGINT, inputHandler); //catch ctrl-c
  while(running) { //busy wait 
  }
  
  printf("Killing threads...\n");
  for (i = 0; i < MAX_THREAD; i++) {
    pthread_kill(tid[i], SIGKILL);
  }
  return 0;
}

