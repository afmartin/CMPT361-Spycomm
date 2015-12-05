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
#include <limits.h>
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
#define FOLDER_PERM 0700

#define MAX_THREAD 8
#define MAX_EVENTS 8

typedef struct _threadArgs{
	int sockfd;
	Box box;
} threadArgs;

static volatile int running = 1;  // A variable used in main that when set to zero will kill all threads and exit the program.

struct _fileInfo {
	char filename[MAX_FILE_NAME];
	long long int fileLen;
	char padID[MD5_STRING_LENGTH];
	uint8_t checksum[MD5_DIGEST_BYTES]; 
};

static pthread_mutex_t mutexlock; // A lock to prevent multiple threads from updating the interface at the same time.

/**
 * printUsage
 *
 * Prints the usage of the program to standard output.
 *
 * Args:
 * char * name - the name of this program
 */
void printUsage(char * name) {
	fprintf(stdout, "Usage: %s [-h] [-p <port number>]\n", name);
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

/**
 * createChallenge
 *
 * Creates a random string of characters to
 * send to the client.
 *
 * Args:
 * char * string - string to write to
 * int len - length of the string
 */
void createChallenge(char * string, int len) {
	char alphaNum[] = "QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890!@#$%^&*()_+-=[]{};':\",.<<>/?";
	for (int i = 0; i < len; i++) {
		string[i] = alphaNum[rand() % (sizeof(alphaNum) - 1)];
	}
}

/**
 * challengeCheck
 *
 * Compares the challenge string sent to and received from client.  If
 * there is any discrepancy it returns -1.
 *
 * Args:
 * uint8_t * challenge - the original challenge string
 * uint8_t * challenger - the challenge received from challenger
 * int len - the length of the challenge (should be same for challenge and challenger)
 *
 * Returns -1 on fail, 1 on pass
 */
int challengeCheck(uint8_t * challenge, uint8_t * challenger, int len) {
	for (int i = 0; i < len; i++){
		if (challenge[i] != challenger[i]){
			fprintf(getLog(), "Client Failed Challenge!");
			return -1;
		}
	}
	return 1;
}

/**
 * getSocket
 *
 * Creates a socket that is bound to the
 * specified port.  (Creates socket suitable
 * for TCP).
 *
 * Returns:
 * int socket descriptor
 *
 * Note: If function fails to make socket descriptor the program will exit.
 */
int getSocket(char * port) {
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

/**
 * initFileTransfer
 *
 * Code to receive a packet from a client to initialize a file
 * transfer with a client.
 *
 * Args:
 * int cd - Client Descriptor
 * fileInfo *info - Pointer to allocated fileInfo, we can write to.
 * long long int * padSize - pointer to write to for OTP size.
 * long long int * padOffset - pointer to write to for padOffset.
 *
 * Returns:
 * constant TRUE on success, constant FALSE on failure
 */
int initFileTransfer(int cd, fileInfo *info, long long int * padSize, long long int * padOffset) {

	//takes client descriptor and struct clientInfo as parameters
	uint8_t buf[MAXLEN];
	memset(buf, 0, MAXLEN);
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
		strncpy(info->filename, temp,  MAX_FILE_NAME);

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
		strncpy(info->padID, temp, MD5_STRING_LENGTH);

		// Copy checksum into the struct
		ptr++;
		for (int i=0; i<MD5_DIGEST_BYTES; i++) {
			info->checksum[i] = *ptr;
			ptr++;
		}
	} else if (buf[0] == 'D') {
		// Client has sent a D packet, and therefore wants to disconnect.
		close(cd);
		fprintf(getLog(), "INFO: Client has disconnected.\n");
		return FALSE;
	} else {
		fprintf(getLog(), "WARNING: Client sent invalid data (%c), closing connection.\n", buf[0]);
		return FALSE;
	}

	// Check if we have the pad, if we have enough room
	getOffsetAndSize(info->padID, padOffset, padSize);

	if (padSize == 0) {
		// if pad size is zero, then the pad is not valid!
		sendError(cd, PAD_INVALID);
		fprintf(getLog(), "ERROR: Pad requested is not valid, ending connection.\n");
		return FALSE;
		// Not enough room in pad
	} else if (info->fileLen > (*padSize - (*padOffset + AUTHENTICATION_LENGTH))) {
		sendError(cd, NO_ROOM);
		fprintf(getLog(), "ERROR: Not enough padd offset for file.    Needed: %lli | Have: %lli\n", info->fileLen, *padOffset);
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
		fprintf(getLog(), "WARNING: Client failed challenge\n");
		sendError(cd, AUTH_NO_MATCH);
		return FALSE;
	}
	//If they did, send an acknowledgement
	else {
		fprintf(getLog(), "INFO: Client pass challenge\n");
		sendAll(cd, (uint8_t *) "A", 1);
		return TRUE;
	}
}

/**
 * getClientAddr
 *
 * Gets the string representation of a sockaddr_storage.
 *
 * Args:
 * struct sockaddr_storage * client - the sockaddr_storage to read from 
 * char * clientAddrString - the string to write to 
 */
void getClientAddr(struct sockaddr_storage * clientAddr, char * clientAddrString) {
	switch (clientAddr->ss_family) {
		case PF_INET:
			if (inet_ntop(clientAddr->ss_family,
						&((struct sockaddr_in *)clientAddr)->sin_addr,
						clientAddrString, INET6_ADDRSTRLEN) == NULL) {
				fprintf(getLog(), "WARNING: Could not convert IPv4 address to string: %s\n", strerror(errno));
				clientAddrString[0] = '\0';
			} else {
				fprintf(getLog(), "INFO: Connected to %s\n", clientAddrString);
			}
			break;
		case PF_INET6:
			if (inet_ntop(clientAddr->ss_family,
						&((struct sockaddr_in6 *)clientAddr)->sin6_addr,
						clientAddrString, INET6_ADDRSTRLEN) == NULL) {
				fprintf(getLog(), "WARNING: Could not convert IPv6 address to string: %s\n", strerror(errno));
				clientAddrString[0] = '\0';
			} else {
				fprintf(getLog(), "INFO: Connected to %s\n", clientAddrString);
			}
			break;
		default:
			fprintf(stderr, "warning: unexpected address family (%d)\n",
					clientAddr->ss_family);
			clientAddrString = "Unknown Address";
			break;
	}

}

/**
 * makeDataFolder
 *
 * Given a folder name, it will add a unique number to append the
 * folder with.  The function will then create the folder.
 *
 * Will only go up to the maximum value of int for unique
 * number appended.
 *
 * Technically it only needs to be at 8, but in case
 * the number of threads is greatly expanded in the future.
 *
 * Args:
 * char * folder - The original folder, which will be modified
 */
void makeDataFolder(char * folder) {
	struct stat st = {0};
	char original[MAX_FOLDER_LEN];

	strncpy(original, folder, MAX_FOLDER_LEN);

	// We need to calculate with the given room in
	// MAX_FOLDER_LEN how many digits we can have added.
	int len = MAX_FOLDER_LEN - strlen(folder) - 1 - 2; // -1 for null term -2 for brackets
	int max = 1;
	for (int i = 0; i <= len; i++) {
		max *= 10;
	}
	// Go one below...
	// Reason for this is best illustrated:
	// Given 3 characters left, we want maximum value to be 999.
	// Previous for loop ends at 1000 which is one extra.
	max -= 1; 

	// But if max is == 0, we have too much room and overflowed the variable.
	// Go to max value of int.
	max = (max == 0 ? INT_MAX : max);

	static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&lock);
	for (long long unsigned i = 0; i < max; i++) {
		char processed[MAX_FOLDER_LEN];
		snprintf(processed, MAX_FOLDER_LEN, "%s(%llu)", original, i);

		if (stat(processed, &st) == -1) {
			mkdir(processed, FOLDER_PERM);
			pthread_mutex_unlock(&lock);
			strncpy(folder, processed, MAX_FOLDER_LEN);
			return;
		}
	}
}

void * worker(void * arg) { //this is the function that threads will call

	threadArgs * ta = (threadArgs *) arg;
	int done = 0;
	int sd = ta->sockfd;
	uint8_t packet[MAXLEN + 1];
	memset(packet, 0 , MAXLEN + 1);
	struct sockaddr_storage clientAddr;
	char clientAddrString[INET6_ADDRSTRLEN] = "";

	char folder[MAX_FOLDER_LEN] = "./serverfiles";
	char t[100]; //time string
	struct stat st = {0};

	// If serverfiles folder does not exist, create it.
	if (stat(folder, &st) == -1) {
		mkdir(folder, FOLDER_PERM);
	}
	strncat(folder, "/", MAX_FOLDER_LEN);

	while (TRUE){
		pthread_mutex_lock(&mutexlock);
		displayWaiting(ta->box);
		pthread_mutex_unlock(&mutexlock);
		int cd = acceptCon(sd, &clientAddr); //wait for a client to connect
		getClientAddr(&clientAddr, clientAddrString);

		getCurrentTime(t);

		strncat(folder, t, MAX_FOLDER_LEN);
		makeDataFolder(folder);	// makeDataFolder will modify folder.
		strcat(folder, "/");
		while (cd) { // full file transfer loop, allows for multiple file transactions
			fileInfo *info = (fileInfo *)malloc(sizeof(fileInfo)); 
			if (info == NULL) {
				fprintf(getLog(), "ERROR: Memory allocation failure: %s\n", strerror(errno));
				sendError(cd, UNSPEC_ERROR);
				closeProgram(true, true);
			}

			long long int padSize = 0;
			long long int padOffset = 0;

			if (initFileTransfer(cd, info, &padSize, &padOffset)) {
				long long int left = info->fileLen;
				fprintf(getLog(), "INFO: Client wants to send a file with %lli bytes\n", info->fileLen);
				getCurrentTime(t);


				// Prepare file destination to write to.
				char fileDest[MAX_FOLDER_LEN];
				memset(fileDest, 0, MAX_FOLDER_LEN);
				strncpy(fileDest, folder, MAX_FOLDER_LEN);
				strncat(fileDest, (*info).filename, MAX_FOLDER_LEN);
				fprintf(getLog(), "INFO: Saving file to: %s\n", fileDest);

				pthread_mutex_lock(&mutexlock);
				clearBox(ta->box);
				connectedToDisplay(ta->box, clientAddrString, info->filename);
				refresh();
				pthread_mutex_unlock(&mutexlock);
				int iterations = info->fileLen/MAXLEN;
				iterations += 1;

				while (!done) { // accepting a single file loop

					//Determines how many bytes we need to receive
					//Either the Max packet length, or whatever is 
					//remaining at the end of the file
					int get;

					if (left < MAX_PACKET_LEN) {
						get = left; 
					} else {
						get = MAX_PACKET_LEN;
					}

					// We have received all of file, confirm checksum
					// if fails, sent the client an error, otherwise send
					// an acknowledgement.
					if (left == 0) {
						uint8_t checksum[MD5_DIGEST_BYTES];
						getMd5DigestFromFile(fileDest, checksum, getFileSizeFromFilename(fileDest));
						if (compareMd5Digest(checksum, info->checksum)) {
							fprintf(getLog(), "INFO: %s checksum matches... Success.\n", info->filename);
							uint8_t ack = A_TYPE;
							sendAll(cd, &ack, 1);	
						} else {
							fprintf(getLog(), "WARNING: %s checksum failed... asking client to retry.\n", info->filename);
							sendError(cd, DATA_INVALID);
						}
						break;
					}

					if (get == 0) {
						int didRecv = recv(cd, packet, 1, 0);
						if (didRecv == -1){
							fprintf(getLog(), "ERROR: Receive failed, ending connection: %s\n", strerror(errno));
							sendError(cd, UNSPEC_ERROR);
							close(cd);
							pthread_exit(NULL);
						}
					}
					//Otherwise, receive as much as we need
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

					//if we receive another packet
					if (packet[0] == (uint8_t) 'F'){
						serverCrypt(packet, 1, info->padID, padOffset, get); 				
						padOffset += get;
						setOffset(info->padID, padOffset);

						//copy the data from the packet into the fileContents
						//And then increment the pointer
						writeToFile(fileDest, packet + 1, get);
						pthread_mutex_lock(&mutexlock);
						progressBar(&(ta->box), iterations);
						refresh();
						pthread_mutex_unlock(&mutexlock);
						left = left - get;
					}
					else {
						fprintf(getLog(), "ERROR: Received erroneous data!\n");
						close(cd);
						break;
					}
					memset(packet, 0, sizeof(packet));
				}    
			} else {
				close(cd);
				break;
			}		
			free(info);
		}
	}
	return NULL;
}

/**
 * inputHandler
 *
 * A function called when a signal interrupt occurs.
 * If CTRL+C (SIGINT) is called, it will set running
 * to zero and cause the program to terminate.
 *
 * Args:
 * int s - the signal
 */
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
	// so we're not using closeProgram 
	fclose(getLog());
	endwin();
	for (i = 0; i < MAX_THREAD; i++) {
		pthread_kill(tid[i], SIGKILL);
	}

	pthread_mutex_destroy(&mutexlock);
	return 1;
}

