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
#include <ncurses.h>
#include <errno.h>
#include "server.h"
#include "file.h"
#include "netCode.h"
#include "crypt.h"
#include "digest.h"
#include "screen.h"
#include "log.h"

#define MAX_FOLDER_LEN 128
#define DONE mvprintw(1, 1, "done\n"); refresh()
#define MAX_CONNECTIONS 10
#define DEFAULT_PORT "36115" // This can change and should be in our protocol
#define SERVER_LOG "spycommd.txt"

#define MAX_THREAD 8
#define MAX_EVENTS 8

typedef struct _threadArgs{
	int sockfd;
	Box box;
} threadArgs;

volatile int running = 1;

struct _fileInfo {
	char filename[MAX_FILE_NAME];
	long long int fileLen;
	char padID[MD5_STRING_LENGTH];
	uint8_t checksum[MD5_DIGEST_BYTES]; 
};

pthread_mutex_t mutexlock;


void printUsage(char* name) {
	fprintf(stdout, "usage: %s [-h] [-p <port number>]\n", name);
	fprintf(stdout, "             h: displays file usage information\n");
	fprintf(stdout, "             p: takes an argument <port number> that will specify"
			"the port number to be used. Default port number is 36115\n");
}

/**
 * sendError
 *
 * Sends error packet to client with
 * specified error number.
 *
 * Args:
 * int cd - client descriptor to send to
 * uint8_t error - error number to use.    Error numbers are defined in netCode.h 
 */
void sendError(int cd, uint8_t error) {
	uint8_t buffer[2] = {'E', error};
	sendAll(cd, buffer, 2);	
}

void createChallenge(char * string, int len){
	char alphaNum[] = "QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890!@#$%^&*()_+-=[]{};':\",.<<>/?";
	for (int i = 0; i < len; i++){
		string[i] = alphaNum[rand() % (sizeof(alphaNum) - 1)];
	}
}

int challengeCheck(uint8_t * challenge, uint8_t * challenger, int len){
	for (int i = 0; i < len; i++){
		if (challenge[i] != challenger[i]){
			fprintf(getLog(), "Client Failed Challenge!");
			return -1;
		}
	}
	return 1;
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
		fprintf(getLog(), "ERROR: %s\n", gai_strerror(num));
		closeProgram(true, true);
	}

	for (i = res; i;    i = i->ai_next) {
		sd = socket(i->ai_family, i->ai_socktype, i->ai_protocol);

		if (sd == -1) {
			fprintf(getLog(), "ERROR: Error creating socket: %s\n", strerror(errno));
			continue;
		}

		/* bind port to socket */
		num = bind(sd, i->ai_addr, i->ai_addrlen);
		if (num == -1) {
			fprintf(getLog(), "ERROR: Error binding port to socket: %s\n", strerror(errno));
			continue;
		}

		/* listen for incoming connections */
		num = listen(sd, MAX_CONNECTIONS);
		if (num == -1) {
			fprintf(getLog(), "ERROR: Error listening for incoming connections: %s\n", strerror(errno));
			continue;
		}

		break;
	}
	if (num == -1 || sd == -1){
		fprintf(getLog(), "ERROR: Could not form a socket: %s\n", strerror(errno));
		freeaddrinfo(res);
		closeProgram(true, true);
	}
	freeaddrinfo(res);
	return sd;
}

int initFileTransfer(int cd, fileInfo *info, long long int * padSize, long long int * padOffset){

	//takes client descriptor and struct clientInfo as parameters
	uint8_t buf[MAXLEN];
	uint8_t * ptr = buf+1;

	char temp[MAXLEN];

	if (recv(cd, buf, MAXLEN, 0) == -1) {
		fprintf(getLog(), "ERROR: Error in read\n");
		sendError(cd, UNSPEC_ERROR);
		close(cd);
		closeProgram(true, true);
	}

	if (buf[0] == T_TYPE) {
		int position = 0;

		// copy filename into struct 
		while (*ptr != '\0') {
			if (position == sizeof(info->filename) - 1){
				break;
			}
			if (*ptr == '\\' || *ptr == '/') {
				memset(temp, 0, sizeof(temp));
				position = 0;
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
				fprintf(getLog(), "ERROR: File size requested too large!\n");
				sendError(cd, UNSPEC_ERROR); // not its own error no because should be impossible to happen.
				close(cd);
				return -1;
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

		// Copy checksum into the struct
		ptr++;
		for (int i=0; i<MD5_DIGEST_BYTES; i++) {
			info->checksum[i] = *ptr;
			ptr++;
		}

		//printf("%s -- %lli -- %s\n", info->filename, info->fileLen, info->padID);
	}
	else return FALSE;
	
	// Check if we have the pad, if we have enough room
	getOffsetAndSize(info->padID, padOffset, padSize);
	
	if (padSize == 0) {
		// if pad size is zero, then the pad is not valid!
		sendError(cd, PAD_INVALID);
		fprintf(getLog(), "ERROR: Pad requested is not valid, ending connection.\n");
		close(cd);
		return FALSE;
		// Not enough room in pad
	} else if (info->fileLen > (*padSize - (*padOffset + AUTHENTICATION_LENGTH))) {
		sendError(cd, NO_ROOM);
		fprintf(getLog(), "ERROR: Not enough padd offset for file.    Needed: %lli | Have: %lli\n", info->fileLen, *padOffset);
		close(cd);
		return FALSE;
	}
	
	//Creates a buffer for the packet to be sent to the client
	char buffer[MAX_FILE_LENGTH_AS_STRING + AUTHENTICATION_LENGTH + 1];
	memset(buffer, 0, MAX_FILE_LENGTH_AS_STRING + AUTHENTICATION_LENGTH + 1);
	snprintf(buffer, MAX_FILE_LENGTH_AS_STRING, "T%lli", *padOffset);
	
	//Creates a challenge string to send for authentication
	char challenge[AUTHENTICATION_LENGTH];
	createChallenge(challenge, AUTHENTICATION_LENGTH);
	
	//Copies the challenge and encrypts it
	char encryptedChallenge[AUTHENTICATION_LENGTH];
	memcpy(encryptedChallenge, challenge, AUTHENTICATION_LENGTH);
	serverCrypt((uint8_t *)encryptedChallenge, 0, info->padID, *padOffset, AUTHENTICATION_LENGTH);
	*padOffset += AUTHENTICATION_LENGTH;
	
	//copy the authentication data into the packet
	memcpy(buffer + MAX_FILE_LENGTH_AS_STRING + 1, encryptedChallenge, AUTHENTICATION_LENGTH);

	//Send all the packet
	int sent = sendAll(cd, (uint8_t *) buffer, MAX_FILE_LENGTH_AS_STRING + AUTHENTICATION_LENGTH + 1);
	if (sent == -1){
		fprintf(getLog(), "ERROR: Error sending challenge!\n");
		return FALSE;
	}
	sleep(1);
	
	//Create an array to store the response
	uint8_t received[AUTHENTICATION_LENGTH + 1];
	int recvRet = recv(cd, received, AUTHENTICATION_LENGTH + 1, 0);
	if (recvRet == -1){
		fprintf(getLog(), "ERROR: Error receiving challenge response!\n");
		return FALSE;
	}
	else if (received[0] != 'T'){
		fprintf(getLog(), "ERROR: Incorrect response to challenge\n");
		sendError(cd, UNSPEC_ERROR);
		return FALSE;
	}
	
	//If we received a 'T' type packet check if they decrypted the challenge
	int pass = challengeCheck((uint8_t *)challenge, received + 1, AUTHENTICATION_LENGTH);
	if (pass < 0){
		fprintf(getLog(), "Client failed challenge\n");
		sendError(cd, AUTH_NO_MATCH);
		return FALSE;
	}
	//If the did, send an acknowledgement
	else {
		fprintf(getLog(), "Client pass challenge\n");
		sendAll(cd, (uint8_t *) "A", 1);
		return TRUE;
	}
}

void getClientAddr(struct sockaddr_storage * client, char* addrString) {
	//struct sockaddr_storage clientAddr = client;
	socklen_t clientLen = sizeof(struct sockaddr_storage);

	int k = getnameinfo((struct sockaddr *)&client, clientLen, addrString, sizeof(addrString), NULL, 0, NI_NUMERICHOST);
	if (k != 0) {
		addrString = "Unknown";
	} 
}

void* worker(void * arg) { //this is the function that threads will call

    threadArgs * ta = (threadArgs *) arg;
    int done = 0;
    int k = 0;
    int sd = ta->sockfd;
    uint8_t packet[MAXLEN + 1];
    memset(packet, 0 , MAXLEN + 1);
    //uint8_t ack = 'A';
    struct sockaddr_storage clientAddr;
    char clientAddrString[INET6_ADDRSTRLEN] = "";
    //uint8_t * fileContents; //+1 to allow for null term
    
    //TODO: No magic numbers
    char folder[MAX_FOLDER_LEN] = "./serverfiles/";
	int baseFolderLen = strlen(folder);
    //char * dir = folder;
    char t[100]; //time string
    struct stat st = {0};
    
    //printf("value of me is %u\n", (unsigned int) pthread_self()); //check thread id			
    while (TRUE){
        pthread_mutex_lock(&mutexlock);
        displayWaiting(ta->box);
        pthread_mutex_unlock(&mutexlock);
        int cd = acceptCon(sd, &clientAddr); //wait for a client to connect
        //getClientAddr(clientAddr, address);
        
        getCurrentTime(t);
        
        // TODO: Use strncat                                                                                                                                        
        strcat(folder, t);
        // TODO: Create serverfiles if DNE
        if (stat(folder, &st) == -1)
            mkdir(folder, 0700);
        strcat(folder, "/");
		int pathBase = strlen(folder);
        
        switch (clientAddr.ss_family) {
            case PF_INET:
				if (inet_ntop(clientAddr.ss_family,
								&((struct sockaddr_in *)&clientAddr)->sin_addr,
								clientAddrString, INET6_ADDRSTRLEN) == NULL) {
					fprintf(getLog(), "WARNING: Could not convert IPv4 address to string: %s\n", strerror(errno));
					clientAddrString[0] = '\0';
				} else {
					fprintf(getLog(), "INFO: Connected to %s\n", clientAddrString);
				}
				break;
            case PF_INET6:
				if (inet_ntop(clientAddr.ss_family,
								&((struct sockaddr_in6 *)&clientAddr)->sin6_addr,
								clientAddrString, INET6_ADDRSTRLEN) == NULL) {
					fprintf(getLog(), "WARNING: Could not convert IPv6 address to string: %s\n", strerror(errno));
					clientAddrString[0] = '\0';
				} else {
					fprintf(getLog(), "INFO: Connected to %s\n", clientAddrString);
				}
				break;
            default:
				//fprintf(stderr, "warning: unexpected address family (%d)\n",
				//	clientAddr.ss_family);
				//clientAddrString = "bad addr";
				break;
        }
        while (cd) { // full file transfer loop, allows for multiple filetrans
            fileInfo *info = (fileInfo *)malloc(sizeof(fileInfo)); 
            if (info == NULL) {
				fprintf(getLog(), "ERROR: Memory allocation failure: %s\n", strerror(errno));
				sendError(cd, UNSPEC_ERROR);
				closeProgram(true, true);
            }
			
            long long int padSize = 0;
			long long int padOffset = 0;
            if (initFileTransfer(cd, info, &padSize, &padOffset)){
				long long int left = info->fileLen;
				fprintf(getLog(), "INFO: Client wants to send a file with %lli bytes\n", info->fileLen);
				getCurrentTime(t);
				
				memset(folder + pathBase, 0, MAX_FOLDER_LEN - baseFolderLen);
				strcat(folder, (*info).filename);
				fprintf(getLog(), "INFO: Saving file to: %s\n", folder);
				//$$printf("folder is %s\n", folder);
				//fileContents = malloc(sizeof(uint8_t) * info->fileLen);
				//uint8_t * ptr = fileContents; // set pointer to start of fileContents

				pthread_mutex_lock(&mutexlock);
				clearBox(ta->box);
				connectedToDisplay(ta->box, clientAddrString, info->filename);
				refresh();
				pthread_mutex_unlock(&mutexlock);
				int iterations = info->fileLen/MAXLEN;
				iterations += 1;
				//iterations == 0 ? iterations = 1 : iterations;
				//for (int i = 0; i < iterations; i++){
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
					if (left == 0){
						uint8_t checksum[MD5_DIGEST_BYTES];
						getMd5DigestFromFile(folder, checksum);
						if (compareMd5Digest(checksum, info->checksum)) {
							uint8_t a = 'A';
							sendAll(cd, &a, 1);	
						} else {
							fprintf(getLog(), "WARNING: %s checksum failed... asking client to retry.\n", info->filename);
							sendError(cd, DATA_INVALID);
						}
						break;
					}
					//Send an acknowledgement so the client knows when it should send data
					//sendAll(cd, &ack, sizeof(ack));
					
					//If we are done receiving, get the 'D' or 'F'
					if (get == 0){
						int didRecv = recv(cd, packet, 1, 0);
						if (didRecv == -1){
							fprintf(getLog(), "ERROR: Receive failed, ending connection: %s\n", strerror(errno));
							sendError(cd, UNSPEC_ERROR);
							close(cd);
							pthread_exit(NULL);
						}
					}
					//Otherwise, recv as much as we need
					else {
						int didRecv = recvAll(cd, get + 1, packet);
						if (didRecv == -1){
							fprintf(getLog(), "ERROR: Receive failed, ending connection: %s\n", strerror(errno));
							
							sendError(cd, UNSPEC_ERROR);
							close(cd);
							pthread_exit(NULL);
						}
						if (didRecv != get + 1){
							fprintf(getLog(), "ERROR: Receive unexpected amount of data, ending connection: %s\n", strerror(errno));
							sendError(cd, UNSPEC_ERROR);
							close(cd);
							pthread_exit(NULL);
						}
					}
					
					//printByteArray(packet);
					
					//if we receive the 'D'
					if(packet[0] == (uint8_t) 'D'){
						//DONE;
						done = 1;
						break;
								}
					//if we receive another packet
					else if (packet[0] == (uint8_t) 'F'){
						serverCrypt(packet, 1, info->padID, padOffset, get); 				
						padOffset += get;
						setOffset(info->padID, padOffset);
						
						//copy the data from the packet into the fileContents
						//And then increment the pointer
						writeToFile(folder, packet + 1, get);
						pthread_mutex_lock(&mutexlock);
						progressBar(&(ta->box), info->fileLen/MAXLEN);
						refresh();
						pthread_mutex_unlock(&mutexlock);
						
						//memcpy(ptr, packet + 1, get);
						left = left - get;//strlen((char*)packet+1);
						//ptr += get;
					}
					else {
						fprintf(getLog(), "ERROR: Received erroneous data!\n");
						close(cd);
						return NULL;
						//$$printf("%s\n", packet);
					}
					memset(packet, 0, sizeof(packet));
				}    
			} 
			else{
				free(info);
				break;
			}		
			if (done) {
				close(cd);
				break;
			}
			free(info);
			//free(fileContents);
		}
		if (k == -1){ //check return value of write to file
			close(cd);
			fprintf(getLog(), "ERROR: Write to File failed!\n");
			pthread_exit(NULL);
		}
	}
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

	initLog(SERVER_LOG);
	loadOffsets();
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
	for (int i = 0; i < MAXBOXES; i++) drawBox(boxes[i]);
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

	fprintf(getLog(), "INFO: Killing threads...\n");
	// At this point in time, it appears the program will randomly exit prematurely when exiting threads.
	// so we're not using closeProgram :(
	fclose(getLog());
	endwin();
	for (i = 0; i < MAX_THREAD; i++) {
		pthread_kill(tid[i], SIGKILL);
	}

	pthread_mutex_destroy(&mutexlock);
	return 1;
}

