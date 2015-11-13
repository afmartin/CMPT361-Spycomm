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

#define IPV6_ADDRLEN 46
#define MAX_PORTS_LEN 32
#define MAX_PATH_LEN 64
#define MD5LEN 16

#define OPTSTRING "hc:p:o:f:"

//Maybe this could be in user.c?
#define DEFAULT_PORT "36115"

#define T_TYPE 'T'
#define F_TYPE 'F'


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

//Build and addrinfo struct linkedlist given an address and port
struct addrinfo * buildAddrInfo (char * address, char * port){
	
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(struct addrinfo));
	
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	int returnCode = getaddrinfo(address, port, &hints, &res);
	
	if (returnCode != 0){
		perror("Getaddrinfo failure: ");
		exit(1);
	}
	
	else{
		return res;
	}
	
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


//takes a socket and an addrinfo struct and attempts to conncet to a remote host
int connectTo (int sock, struct addrinfo * info){
	
	int returnCode = connect(sock, (struct sockaddr *) info->ai_addr, info->ai_addrlen);
	
	if (returnCode != 0){
		fprintf(stderr, "Failed to connect!/n");
		return sock;
	}
	
	return sock;
}


//Given a uint8_t array sends the array until all the data is sent,
//or something goes wrong. Takes a socket, the buffer, the length of buffer
//and returns a boolean for success, and sets the len variable to the 
//amount of bytes actually sent
int sendAll(int sock, uint8_t * buffer, int  * len){
	
	int sent = 0;
	int left = *len;
	int ret;
		
	while (sent < *len){
		ret = send(sock, buffer + sent, left, 0);
		if (ret == -1){ return -1; }
		sent += ret;
		left -= ret;
	}
	
	*len = sent;
	
	return 0;
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
	memcpy(position, length, strlen(fileName) + 1);
	position += strlen(fileName) + 1;
	memcpy(position, padID, strlen(fileName) + 1);
	
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
	
	initiateFileTransfer(sock, "FILE1", "1", "S8267SHASKDJHKAD");
	
	close(sock);
	
	freeaddrinfo(serverInfo);

	return 1;
}