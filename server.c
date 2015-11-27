/*
  #################################################################################
  CMPT 361 - Assignment 3                                                         
  Group 4: Nick, John, Alex, Kevin
  November 6th, 2015
  Filename: server.c Description:
  #################################################################################
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <getopt.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <ncurses/ncurses.h>
#include "server.h"
#include "file.h"
#include "netCode.h"
#include "crypt.h"
#include "digest.h"
#include "screen.h"

#define DONE mvprintw(1, 1, "done\n"); refresh()
#define MAX_CONNECTIONS 10
#define DEFAULT_PORT "36115" // This can change and should be in our protocol

#define MAX_THREAD 8
#define MAX_EVENTS 8

typedef struct _threadArgs{
  
  int sockfd;
  Box box;
} threadArgs;

volatile int running = 1;

typedef struct _fileInfo {
  char filename[MAX_FILE_NAME];
  long long int fileLen;
  char padID[MD5_STRING_LENGTH];
} fileInfo;

pthread_mutex_t mutexlock;


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
    info->fileLen = atoll(temp);
	
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
	
    //printf("%s -- %lli -- %s\n", info->filename, info->fileLen, info->padID);
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

void getClientAddr(struct sockaddr_storage client, char* addrString) {
  char clientAddrString[INET6_ADDRSTRLEN] = "";
  struct sockaddr_storage clientAddr = client;
  socklen_t clientLen = sizeof(struct sockaddr_storage);

  int k = getnameinfo((struct sockaddr *)&clientAddr, clientLen, addrString, sizeof(addrString), NULL, NULL, NI_NUMERICHOST);
  if (k != 0) {
    addrString = "Unknown";
  } 
}

void* worker(void * arg) { //this is the function that threads will call
  
  //DONE;
  threadArgs * ta = (threadArgs *) arg;
  int done = 0;
  int k = 0;
  //uint8_t* temp;
  //char* filePath;
  //DONE;
  int sd = ta->sockfd;
  //DONE;
  uint8_t packet[MAXLEN + 1];
  memset(packet, 0 , MAXLEN + 1);
  //size_t len = MAXLEN;
  uint8_t ack = 'A';
  struct sockaddr_storage clientAddr;
  uint8_t * fileContents; //+1 to allow for null term

  //TODO: No magic numbers
  char folder[100] = "./serverfiles/";
  char t[100]; //time string
  struct stat st = {0};

  //printf("value of me is %u\n", (unsigned int) pthread_self()); //check thread id			
  while (TRUE){
    pthread_mutex_lock(&mutexlock);
    displayWaiting(ta->box);
    pthread_mutex_unlock(&mutexlock);
    int cd = acceptCon(sd, &clientAddr); //wait for a client to connect
    char address[100];
    getClientAddr(clientAddr, address);
    
    while (cd) { // full file transfer loop, allows for multiple filetrans
      fileInfo *info = (fileInfo *)malloc(sizeof(fileInfo)); 
      if (info == NULL) {
	fprintf(stderr, "Memory allocation failure\n");
	exit(1);
      }
      
      getCurrentTime(t);

      // TODO: Use strncat                                                                                 
      strcat(folder, t);
      // TODO: Create serverfiles if DNE.                                                                  
      if (stat(folder, &st) == -1)
	mkdir(folder, 0700);
      strcat(folder, "/");
      
      
      if (initFileTransfer(cd, info)){
	long long int left = info->fileLen;
	connectedToDisplay(ta->box, address, info->filename);
	getCurrentTime(t);

	strcat(folder, (*info).filename);
	//$$printf("folder is %s\n", folder);
	//fileContents = malloc(sizeof(uint8_t) * info->fileLen);
	//uint8_t * ptr = fileContents; // set pointer to start of fileContents
    
    // Check if we have the pad, if we have enough room
    long long int padSize = 0;
    long long int padOffset = 0;

    getOffsetAndSize(info->padID, &padOffset, &padSize);

    if (padSize == 0) {
        sendAll(cd, "E2", 2);
        return NULL;
        // Not enough room in pad
    } else if (info->fileLen > (padSize - padOffset)) {
        sendAll(cd, "E1", 2);
        return NULL;
    }

    char buffer[MAX_FILE_LENGTH_AS_STRING + 1];
    memset(buffer, 0, MAX_FILE_LENGTH_AS_STRING + 1);
    snprintf(buffer, MAX_FILE_LENGTH_AS_STRING, "T%lli", padOffset);



	sendAll(cd, (uint8_t *) buffer, MAX_FILE_LENGTH_AS_STRING + 1);
	pthread_mutex_lock(&mutexlock);
	connectedToDisplay(ta->box, "192.168.0.15", info->filename);
	refresh();
	pthread_mutex_unlock(&mutexlock);
	int iterations = info->fileLen/MAXLEN;
	iterations += 1;
	//iterations == 0 ? iterations = 1 : iterations;
	while(!done){ // accepting a single file loop
					
	  //Determines how many bytes we need to receive
	  //Either the Max packet length, or whatever is 
	  //remaining at the end of the file
	  int get;
	  //left = info->fileLen - (ptr - fileContents);
	  //$$printf("\n value of left is %lli\n", left);
	  if (left < MAXLEN + 1){
	    get = left;
	  }
	  else {
	    get = MAXLEN;
	  }
					
	  //Send an acknowledgement so the client knows when it should send data
	  //sendAll(cd, &ack, sizeof(ack));
					
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
	    //$$$printf("Encrypted: %s\n\n", packet + 1);
	    //DONE;
	    //getMd5Digest(packet+1, info->fileLen, temp);
	    //convertMd5ToString(filePath, temp);
	    //strcat(folder, filePath); //concat file path with md5 digest

	    serverCrypt(packet, 1, info->padID, padOffset, get); 				
	    padOffset += get;
	    setOffset(info->padID, padOffset);

		//$$$printf("Decrypted: %s\n\n", packet + 1);
	    //copy the data from the packet into the fileContents
	    //And then increment the pointer
	    writeToFile(folder, packet + 1, get);
	    pthread_mutex_lock(&mutexlock);
	    progressBar(&(ta->box), info->fileLen/MAXLEN);
	    refresh();
	    pthread_mutex_unlock(&mutexlock);
	    if (ta->box.state == 0) {
	      printf("a");
	      sleep(3);
	    }
	    //memcpy(ptr, packet + 1, get);
	    left = left - get;//strlen((char*)packet+1);
	    //ptr += get;
	  }
	  else {
	    printf("Received erroneous data!\n");
	    //$$printf("%s\n", packet);
	  }
	  memset(packet, 0, sizeof(packet));
	}
      } else{
	free(info);
	break;
      }
      //Add a folder prefix
      //char folder[100] = "./serverfiles/";
      //strcat(folder, (*info).filename);
			
      //Open a file and write the fileContents to it
      //FILE * fp = fopen(folder, "wb+");
      //k = fwrite(fileContents, 1, info->fileLen + 1, fp);
      //      fclose(fp);
			
      if (done) {
	close(cd);
	break;
      }
      free(info);
      //free(fileContents);
    }
    if (k == -1){ //check return value of write to file
      close(cd);
      printf("Write to File failed!\n");
      pthread_exit(NULL);
    }
    //$$printf("Hello I'm thread %u and I've finished with client %d\n", (unsigned int) pthread_self(), cd );
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
  
  threadArgs args[MAX_THREAD];

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
  initscr();
  noecho();
  Box boxes[MAXBOXES];
  int y = 2;
  for (int i = 0; i< MAXBOXES; i+=2){
    Box box1 = {.boxNo = i, .progress = 0, .state = 0,
                .row = y,
                .column = COLUMN1};
    Box box2 = {.boxNo = i+1, .progress = 0, .state = 0,
                .row = y,
                .column = COLUMN2};
    boxes[i] = box1;
    boxes[i+1] = box2;
    y += HEIGHT+1;
  }
  for (int i = 0; i < MAXBOXES; i++)
    drawBox(boxes[i]);
  refresh();
  
  sd = getSocket(port); //This should be in netCode.h
  pthread_mutex_init(&mutexlock, NULL);
  for (i = 0; i < MAX_THREAD; i++) { //create threads
    args[i].sockfd = sd;
    args[i].box = boxes[i];
    pthread_create(&tid[i], NULL, worker, &args[i]);
    
  }
  
  signal(SIGINT, inputHandler); //catch ctrl-c
  while(running) { //busy wait 
    
  }
  
  printf("Killing threads...\n");
  for (i = 0; i < MAX_THREAD; i++) {
    
    pthread_kill(tid[i], SIGKILL);
  }
  
  pthread_mutex_destroy(&mutexlock);
  //  pthread_exit(NULL);
  endwin();
  return 0;
}

