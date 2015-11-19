
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

#define MAX_FILE_LENGTH_AS_STRING 10

#define OPTSTRING "hc:p:o:"

#define USAGE "Usage: %s [-h] -c \"SERVERADDRESS\" -p \"PORTS_TO_KNOCK\" -o \"PATH_TO_OTP\" file1 file2 file3\n", argv[0]

#define DONE   printf("Done!\n");

//This is a struct to hold the command line strings
struct commandLine {
  char address[IPV6_ADDRLEN];
  char ports[MAX_PORTS_LEN];
  char padPath[MAX_PATH_LEN];
  char filePath[MAX_FILES_LEN];
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
	int count = argc - 1;
  
	while((opt = getopt(argc, argv, OPTSTRING)) != -1){
		switch (opt){
			case 'h':
				printf(USAGE);
				exit(0);
				break;
			case 'c':
				strncpy(options->address, optarg, IPV6_ADDRLEN);
				count -= 2;
				break;
			case 'p':
				strncpy(options->ports, optarg, MAX_PORTS_LEN);
				count -= 2;
				break;
			case 'o':
				strncpy(options->padPath, optarg, MAX_PATH_LEN);
				count -= 2;
				break;
			default:
				printf(USAGE);
				exit(0);
				break;
		}
	}
  
    if(count <= 0){
	    printf("No files listed!\n");
	    printf(USAGE);
    }
	
	memset(options->filePath, 0, MAX_FILES_LEN);
	
	int argvPos = optind;
	int filePathPos = 0;
  
	while (count != 0){
		if (filePathPos + strlen(argv[argvPos]) + 1 > MAX_FILES_LEN){
			printf("Too Many files chosen!\n");
			exit(0);
		}
		strncpy(&(options->filePath[filePathPos]), argv[argvPos], MAX_PATH_LEN);
		filePathPos += strlen(argv[argvPos]) + 1;
		argvPos++;
		count--;
	}
	options->filePath[++filePathPos] = '\0';
  
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
	
	/* //printf("%d\n",initStringLen);
	for(int i = 0; i < initStringLen; i++){
		printf("%c", initString[i]);
	}
	printf("\n"); */
	
	
	//Create a int to hold the length of bytes to send
	int sending = initStringLen;
	
	//Attempts to send all the data out
	int sent = sendAll(sock, (uint8_t *) initString, sending); 
	
	//If sendAll failed
	if (sent == -1){
		fprintf(stderr, "Sendall Failed!\n");
		exit(1);
	}

	//Check to see if all the data sent
	if (sent != sending){
		fprintf(stderr, "Failed to send all the data!\n");
		return 0;
	}
	//if it did all send, return true
	else {
		return 1;
	}
}

void sendFile (char * address, char * port, char * fileName, char * hash){

	struct addrinfo * serverInfo = buildAddrInfo(address, port);

	//Attempts to grab a new socket
	int sock = getSocket(serverInfo);
	int check = connectTo(sock, serverInfo);
	if (check == -1){
		fprintf(stderr, "Cannot connect to server!\n");
		exit(0);
	}
	
	//Get info about the file to send
	FILE * fp = fopen(fileName, "r");
	int fd = fileno(fp);
	int fileSize = getFileSize(fd);
	char fileLenAsString[MAX_FILE_LENGTH_AS_STRING];
	snprintf(fileLenAsString, MAX_FILE_LENGTH_AS_STRING, "%d", fileSize);
	
	//Sends the initialization data
	initiateFileTransfer(sock, fileName, fileLenAsString, hash);
	
	
	//Waits for an acknowledgement before sending data
	//to ensure the server is ready to receive
	uint8_t wait;
	int checkRet;
	checkRet = recv(sock, &wait, 1, 0);
	if (checkRet == -1 || wait != (uint8_t) 'A'){
		perror("-1?");
		printf("Protocol Error! %d \n", wait);
		exit(1);
	}
	
	sleep(1);
	
	//Create the buffer for the Packet to be sent
	uint8_t buffer[MAX_PACKET_LEN + 1];
	memset(buffer, 0, MAX_PACKET_LEN + 1);
	
	//Send out as many packets as there is data
	for (int i = 0; i <= fileSize / (MAX_PACKET_LEN); i++){
		buffer[0] = 'F';
		size_t read = fread(buffer + 1, 1, MAX_PACKET_LEN, fp);
		if (read == -1){
			fprintf(stderr, "An Error occured reading %s\n", fileName);
			exit(0);
		}
		
		int sent = sendAll(sock, buffer, read + 1);
		if (sent == -1){
			printf("Failure to send!\n");
			exit(1);
		}
	}
	close(fd);
	
	//Send a 'D' to indicate file transfer completion
	uint8_t done = 'D';	
	sendAll(sock, &done, sizeof(uint8_t));
	
	
	//Wait for an acknowledgement before closing the connection
	checkRet = recv(sock, &wait, sizeof(uint8_t), 0);
	if (checkRet == -1 || wait != (uint8_t) 'A'){
		perror("-1?");
		printf("Protocol Error! %d \n", wait);
		exit(1);
	}
	
	close(sock);
	freeaddrinfo(serverInfo);

}

int main (int argc, char * argv[]){

	struct commandLine * opts = getOptions(argc, argv);
	
	printf("Command line opts were: %s %s %s\n", opts->address, opts->ports, opts->padPath);
  
	sendFile(opts->address, opts->ports, opts->filePath, "PLEASEWORK!!!!!!");
	
	return 0;
}
