
/*

VERY INSIGHTFUL AND INORMATIVE COMMENT BLOCK GOES HERE

*/	

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "file.h"
#include "netCode.h"

#define OPTSTRING "hc:p:o:f:"

#define DONE   printf("Done!\n");

//This is a struct to hold the command line strings
struct commandLine {
  char address[IPV6_ADDRLEN];
  char ports[MAX_PORTS_LEN];
  char padPath[MAX_PATH_LEN];
  char filePath[MAX_PATH_LEN];
};

//Grabs the command line options and places them into their
//coresponding parts of the commandLine Struct
struct commandLine * getOptions (int argc, char * argv[]){
  struct commandLine * options = malloc(sizeof(struct commandLine));
  
  if (options == NULL){
    printf("Memory allocation failed!\n");
    exit(1);
  }
  
  int opt;
  
  while((opt = getopt(argc, argv, OPTSTRING)) != -1){
    switch (opt){
    case 'h':
      printf("Usage: %s [-h] -c \"SERVERADDRESS\" -p \"PORTS_TO_KNOCK\" -o \"PATH_TO_OTP\"\n", argv[0]);
      exit(0);
      break;
    case 'c':
      strncpy(options->address, optarg, IPV6_ADDRLEN);
      break;
    case 'p':
      strncpy(options->ports, optarg, MAX_PORTS_LEN);
      break;
    case 'o':
      strncpy(options->padPath, optarg, MAX_PATH_LEN);
      break;
    case 'f':
      strncpy(options->filePath, optarg, MAX_PATH_LEN);
      break;
    default:
      printf("Usage: %s [-h] -c \"SERVERADDRESS\" -p \"PORTS_TO_KNOCK\" -o \"PATH_TO_OTP\"\n", argv[0]);
      exit(0);
      break;
    }
  }
  
  return options;
}

//Gets a new socket given an addrinfo struct linkedlist
int getSocket (struct addrinfo * info){
	
	struct addrinfo * iter;
	
	int sock = -1;
	
	//iterates through the list and attempts to make a socket
	for (iter = info; iter; iter=iter->ai_next){
	  sock =  socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);
	  if (sock == -1){
	    perror("Socket creation: ");
	    iter = iter->ai_next;
	    continue;
	  }
	  break;
	}
	
	if (iter == NULL && sock == -1){
	  fprintf(stderr, "Failed to make a socket!/n");
	  exit(1);
	}
	
	return sock;
}

//Sends the initialization to the server given the socket, filename, length of file,
//MD5 padID, and returns true or false depending on whether it succeeds
int initiateFileTransfer(int sock, char * fileName, char * length, char * padID){
	
	//Get the length of the initialization packet
	int initStringLen = sizeof(char) + strlen(fileName) + strlen(length) + strlen(padID) + 3;
	
	//Create an array for the packet
	char initString[initStringLen];
	memset(&initString, 0, initStringLen);
	
	//Build the packet by concatenating the strings
	initString[0] = T_TYPE;
	char * position = &initString[1];
	memcpy(position, fileName, strlen(fileName) + 1);
	position += strlen(fileName) + 1;
	memcpy(position, length, strlen(length) + 1);
	position += strlen(length) + 1;
	memcpy(position, padID, strlen(padID) + 1);
	
	//printf("%d\n",initStringLen);
	for(int i = 0; i < initStringLen; i++){
		printf("%c", initString[i]);
	}
	printf("\n");
	
	
	//Create a int to hold the length of bytes to send
	//Will also hold the number of bytes sent after sendAll
	int * sending = malloc(sizeof(int));
	*sending = sizeof(initString);

	//A check variable to see if the sent length is equal to
	//actual length after the sendAll call
	int check = initStringLen;
	
	//Attempts to send all the data out
	int sent = sendAll(sock, (uint8_t *) initString, sending); 
	
	//If sendAll failed
	if (sent == -1){
		fprintf(stderr, "Sendall Failed!\n");
		exit(1);
	}

	//Check to see if all the data sent
	if (*sending != check){
		fprintf(stderr, "Failed to send all the data!\n");
		free(sending);
		return 0;
	}
	//if it did all sent, return true
	else {
		free(sending);
		return 1;
	}
}

int main (int argc, char * argv[]){

  struct commandLine * opts = getOptions(argc, argv);
  
  printf("Command line opts were: %s %s %s\n", opts->address, opts->ports, opts->padPath);
  
  struct addrinfo * serverInfo = buildAddrInfo(opts->address, opts->ports);
  
  int sock = getSocket(serverInfo);
  
  printf("Got a socket!\n");
  
  connectTo(sock, serverInfo);
  
  
  
  
  FILE * fp = fopen(opts->filePath, "r");
  int fd;
  fd = fileno(fp);
  int fileSize;
  fileSize = getFileSize(fd);
  printf("%d\n", fileSize);
  char fileLenAsString[10];
  snprintf(fileLenAsString, 10, "%d", fileSize);
  
  initiateFileTransfer(sock, opts->filePath, fileLenAsString, "S8267SHASKDJHKAD");
  printf("Got Here!\n");
  uint8_t  ** fileData = getFileArray(fp, fileSize);
  printf("Got the file data!\n");
  close(fd);
  fp = NULL;
	
  //char * type = malloc(1);
  //*type = 'F';
  
  int sent = 1;
  
  int numberOfPackets = fileSize / MAX_PACKET_LEN ;
  printf("%d\n", numberOfPackets);
  uint8_t * data = malloc(MAX_PACKET_LEN + 1);
  data[0] = 'F';
  /* //data[0] = (*(uint8_t *) type); */
  /* memcpy(data + 1, *fileData, MAX_PACKET_LEN); */
  /* *sent = MAX_PACKET_LEN; */
  /* sendAll(sock, data, sent); */
  

  for (int i = 0; i < numberOfPackets + 1; i++){
    printf("Loop!\n");
    data[0] = 'F';
    memcpy(data+1, fileData[i], MAX_PACKET_LEN);
    //printByteArray(fileData[i]);
    sent = MAX_PACKET_LEN;
    sleep(1);
    sendAll(sock, data, &sent);
  }

  sent = 1;
  sendAll(sock, (uint8_t *)  "D", &sent);
  //printByteArray(fileData, fileSize + 1);
  
  /* for (int i = 0; i < numberOfPackets; i++) */
  /*   free(fileData[i]); */
  
  close(sock);
  freeaddrinfo(serverInfo);
  
  return 0;
}
