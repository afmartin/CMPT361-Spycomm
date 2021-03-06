/*
#################################################################################
CMPT 361 - Assignment 3                                                                                                                 
Group 4: Nick, John, Alex, Kevin
November 6th, 2015
Filename: client.c 
Description: This is a client program implementation 
of the Legendary File Transfer Protocol documented at
within an email you should have received with this program.
As a Spy for the Imperium of Awesome, you will be using this 
program to securely transfer data you have collected during your
counter intelligence efforts. You were securely sent a One-Time-Pad
which you will use with this program to encrypt the data that will be sent.
Use it wisely. The pad is limited and will eventually be used up. Only
send the most important data back to HQ. 
#################################################################################
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h> 
#include <errno.h>
#include <sys/stat.h>

#include "file.h"
#include "netCode.h"
#include "digest.h"
#include "crypt.h"
#include "log.h"

//This defines the String used by getopt to determine the command line arguments
#define OPTSTRING "hc:p:o:"

//This defines the string to be used when printing the usage
#define USAGE "USAGE: %s [-h] -c \"SERVERADDRESS\" -p \"PORT\" -o \"PATH_TO_OTP\" filenames...\n", argv[0]

//This defines the name of the client's log file
#define CLIENT_LOG "spycomm.txt"

//This is a struct to hold the command line strings
//It stores the server address, the port to connect on, 
//as well as the path to the pad, and the number of files
struct commandLine {
	char address[IPV6_ADDRLEN];
	char ports[MAX_PORTS_LEN];
	char padPath[MAX_PATH_LEN];
	int fileNum;
};



/**
 * getOptions
 *
 * Grabs the command line options and places them into their
 * corresponding parts of the commandLine Struct
 *
 * Args:
 * int argc: The total number of command line arguments
 * char * argv : The double pointer of the command line arguments
 *
 * Returns:
 * A commandLine struct containing the commandline options parsed 
 */
struct commandLine * getOptions (int argc, char * argv[]){
	struct commandLine * options = malloc(sizeof(struct commandLine));

	if (options == NULL){
		fprintf(getLog(), "ERROR: Memory allocation failed!\n");
		closeProgram(true, false);
	}

	int opt;
	//Count holds the number of arguments and is decremented
	//after a option in parsed
	int count = argc - 1;

	while((opt = getopt(argc, argv, OPTSTRING)) != -1){
		switch (opt){
			case 'h':
				fprintf(stdout, USAGE);
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
				// Checks to see if file exists.
				FILE *f;
				if ((f = fopen(options->padPath, "rb")) == NULL) {
					fprintf(stderr, "ERROR: Not valid path to one time pad.\n");
					exit(1);
				}
				fclose(f);
				break;
			default:
				fprintf(stdout, USAGE);
				exit(1);
				break;
		}
	}

	
	//if there are no arguments left, then the were no files listed
	if(count <= 0){
		fprintf(stderr,"ERROR: No files listed!\n");
		fprintf(stdout, USAGE);
		exit(1);
	}
	
	options->fileNum = count;
	
	return options;
}

/**
 * getSocket
 *
 * Creates a socket ready for a connect call 
 * given a addrinfo struct
 *
 * Args:
 * struct addrinfo *info: A struct containing the server's connection info
 *
 * Returns:
 * An integer indicating the success or failure
 */
int getSocket (struct addrinfo * info){

	struct addrinfo * iter;

	int sock = -1;

	//iterates through the list and attempts to make a socket
	for (iter = info; iter; iter=iter->ai_next){
		sock =  socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);
		if (sock == -1){
			fprintf(getLog(), "WARNING: Failed socket creation attempt: %s/n", strerror(errno));
			iter = iter->ai_next;
			continue;
		}
		break;
	}

	if (iter == NULL && sock == -1){
		fprintf(getLog(), "ERROR: Failed to make a socket: %s/n", strerror(errno));
		closeProgram(true, false);
	}

	return sock;
}

/**
 * printErrorThenExit
 *
 * Parses the received error code then prints the corresponding
 * error string to the log and terminates
 *
 * Args:
 * uint8_t errorCode: The 8 bytes error code received
 */
void printErrorThenExit(uint8_t errorCode){
	switch (errorCode){
		case NO_ROOM:
			fprintf(getLog(), "ERROR: The server indicates insufficient pad is remaining\n");
			closeProgram(true, false);
			break;
		case PAD_INVALID:
			fprintf(getLog(), "ERROR: The server indicates that an invalid pad was used\n");
			closeProgram(true, false);
			break;
		case DATA_INVALID:
			fprintf(getLog(), "ERROR: The server indicates that invalid data was sent\n");
			closeProgram(true, false);
			break;
		case AUTH_NO_MATCH:
			fprintf(getLog(), "ERROR: The server indicates that authentication failed\n");
			closeProgram(true, false);
			break;
		case CON_HANG:
			fprintf(getLog(), "ERROR: The server indicates that the connection was lost\n");
			closeProgram(true, false);
			break;
		case UNSPEC_ERROR:
			fprintf(getLog(), "ERROR: The server indicates that unspecified error occurred\n");
			closeProgram(true, false);
			break;
		case IN_USE:
			fprintf(getLog(), "ERROR: The server indicates that the pad is in use\n");
			closeProgram(true, false);
			break;
		default:
			fprintf(getLog(), "WARNING: UNKNOWN ERROR CODE RECEIVED\n");
			closeProgram(true, false);
			break;
	}
}

/**
 * initiateFileTransfer
 *
 * Handles the handshaking process with the server
 *
 * Args:
 * int sock: A socket descriptor with an accepted TCP connected to the server
 * char * fileName: The string containing the file to send's file name
 * char * length: The string containing the length of the file as a string
 * char * padPath: The string containing the path to the OTP to use
 * char * digestStr: The string containing the digest of OTP we are using
 *
 * Returns:
 * The offset within the OTP we should use for the file transfer as long long int
 * 
 */
long long int initiateFileTransfer(int sock, char * fileName, char * length, char * padPath, char * digestStr){
	
	
	// Compute digest of entire file.
	uint8_t fileDigest[MD5_DIGEST_BYTES]; 
	getMd5DigestFromFile(fileName, fileDigest, atoll(length));
	
	
	//Get the length of the initialization packet
	int initStringLen = sizeof(char) + strlen(fileName) + strlen(length) + strlen(digestStr) + MD5_DIGEST_BYTES + 3;

	
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
	memcpy(position, digestStr, strlen(digestStr) + 1);
	position += strlen(digestStr) + 1;
	memcpy(position, fileDigest, MD5_DIGEST_BYTES);


	//Create a int to hold the length of bytes to send
	int sending = initStringLen;

	//Attempts to send all the data out
	int sent = sendAll(sock, (uint8_t *) initString, sending); 

	//If sendAll failed
	if (sent == -1){
		fprintf(getLog(), "ERROR: Sending data failed: %s\n", strerror(errno));
		closeProgram(true, false);
	}

	//Check to see if all the data sent
	if (sent != sending){
		fprintf(getLog(), "ERROR: Failed to send all the data!\n");
		closeProgram(true, false);
	}
	
	//Waits for an acknowledgement before sending data
	//to ensure the server is ready to receive
	uint8_t wait[1 + MAX_FILE_LENGTH_AS_STRING + AUTHENTICATION_LENGTH];
	int checkRet;
	checkRet = recv(sock, wait, 1 + MAX_FILE_LENGTH_AS_STRING + AUTHENTICATION_LENGTH, 0);
	if (checkRet == -1){
		fprintf(getLog(), "ERROR: Receive failed while waiting for an acknowledgement\n");
		closeProgram(true, false);
	}
	else if (wait[0] != 'T'){
		if (wait[0] == 'E'){
			printErrorThenExit(wait[1]);
		}
		else {
			fprintf(getLog(), "ERROR: Expected a 'T' or 'E' type, but received '%c'\n", wait[0]);
			printf("%s\n", wait);
			closeProgram(true, false);
		}
	}
	
	//Make sure we received all of the handshaking data
	if (checkRet != sizeof(wait)){
		int returnValue = recvAll(sock, sizeof(wait) - checkRet, wait + checkRet);
		if (returnValue == -1){
			fprintf(getLog(), "ERROR: Receive failed while waiting for handshake\n");
			closeProgram(true, false);
		}
	}
	
	printf("Received authentication challenge\n");
	
	
	//get the offset from the server packet
	char offsetString[MAX_FILE_LENGTH_AS_STRING];
	memcpy(offsetString, wait + 1, MAX_FILE_LENGTH_AS_STRING);
	long long int offset = atoll(offsetString);
	
	//create a pointer to the authentication portion
	uint8_t * auth = wait + 1 + MAX_FILE_LENGTH_AS_STRING; 
	//Decrpyt the authentication data
	printf("Decrypting now........\n");
	clientCrypt(auth, 0, padPath, offset, AUTHENTICATION_LENGTH);
	
	//create a packet to send back to the server
	uint8_t authenticationString[1 + AUTHENTICATION_LENGTH];
	authenticationString[0] = 'T';
	memcpy(&authenticationString[1], auth, AUTHENTICATION_LENGTH);
	
	//Send the challenge response
	printf("Sending challenge response...\n");
	sent = sendAll(sock, (uint8_t *) authenticationString, AUTHENTICATION_LENGTH + 1);
	if (sent == -1){
		fprintf(getLog(), "ERROR: Sending data failed: %s\n", strerror(errno));
		closeProgram(true, false);
	}
	
	//Wait for either an acknowledgement or Error
	checkRet = recv(sock, wait, sizeof(wait), 0);
	if (checkRet == -1){
		fprintf(getLog(), "ERROR: Receive failed while waiting for an acknowledgement\n");
		closeProgram(true, false);
	}
	else if (wait[0] != 'A'){
		if (wait[0] == 'E'){
			printErrorThenExit(wait[1]);
		}
		else {
			fprintf(getLog(), "ERROR: Expected a 'T' or 'E' type, but received '%c'\n", wait[0]);
			closeProgram(true, false);
		}
	}
	printf("We are authenticated!\n");
	printf("Transferring........\n\n");
	return offset + AUTHENTICATION_LENGTH;
}


/**
 * isAFile
 *
 * Check whether a filepath is a file or directory
 *
 * Args:
 * char * filePath: A string containing a path
 *
 * Returns:
 * A integer (boolean) of whether the path was a file
 */
int isAFile(char * filepath){
	struct stat path_stat;
	stat(filepath, &path_stat);
	return S_ISREG(path_stat.st_mode);
}


/**
 * sendFile
 *
 * Handles the entire file transfer
 *
 * Args:
 * char * address: The IP address of the server as a string
 * char * port: The port which the client should connect to
 * char * fileName: The file name of the file to be  send 
 * char * padPath: The file path to the one time pad to use
 * char * digestStr: The digest of the OTP to use
 * int sock: The socket Descriptor which has been 'connect'ed to the server
 *
 * Returns:
 * A integer (boolean) to indicate the success state
 */
int sendFile (char * address, char * port, char * fileName, char * padPath, char * digestStr, int sock){

	if (!isAFile(fileName)){
		return -1;
	}

	//Get info about the file to send
	FILE * fp = fopen(fileName, "r");
	if (fp == NULL){
		return -1;
	}
	
	
	
	//Creates a file descriptor for the file and calculates
	//the offset and converts to string
	int fd = fileno(fp);
	long long int fileSize = getFileSize(fd);
	char fileLenAsString[MAX_FILE_LENGTH_AS_STRING];
	snprintf(fileLenAsString, MAX_FILE_LENGTH_AS_STRING, "%lli", fileSize);

	if(fileSize == 0){
		fprintf(stdout, "File is empty!");
		return -1;
	}	

	fprintf(getLog(), "INFO: Sending file: %s of size %sB\n", fileName, fileLenAsString); 
	
	//Sends the initialization data and received the offset to use
	long long int offset = initiateFileTransfer(sock, fileName, fileLenAsString, padPath, digestStr);
	if (offset == -1){
		fprintf(getLog(), "ERROR: Error handshaking: program terminating\n");
		closeProgram(true, false);
	}

	//Create the buffer for the Packet to be sent
	uint8_t buffer[MAX_PACKET_LEN + 1];

	//Used for the width of the command line progress bar
	int barWidth = 60;

	for (int i = 0; i <= fileSize / (MAX_PACKET_LEN); i++){
		memset(buffer, 0, MAX_PACKET_LEN + 1);
		buffer[0] = 'F';
		int sentSoFar = i * MAX_PACKET_LEN;
		int amountToSend = (fileSize - sentSoFar < MAX_PACKET_LEN ? fileSize - sentSoFar : MAX_PACKET_LEN);
		size_t read = fread(buffer + 1, 1, amountToSend, fp);
		if (read == -1){
			fprintf(getLog(), "ERROR: An Error occurred reading %s\n", fileName);
			closeProgram(true, false);
		}
		
		//Encrypt the buffer and update the pad offset
		clientCrypt(buffer, 1, padPath, offset, read + 1);
		offset += read;

		//Sent all the data and check for errors
		int sent = sendAll(sock, buffer, read + 1);
		if (sent == -1){
			fprintf(getLog(), "ERROR: Failed to send: %s\n", strerror(errno));
			closeProgram(true, false);
		}

		int percent = (int)(ceil((float)((i + 1) * 100) / ceil((float)fileSize / (MAX_PACKET_LEN))));

		//Prints the command line progress bar
		printf("%2d%% [", percent);
		for (int j = 0; j < ((percent * barWidth) / 100); j++){
			printf("=");
		}
		for (int j = 0; j < (((100 - percent) * barWidth) / 100); j++){
			printf(" ");
		}
		printf("]\n%dB Sent              \r\b\r", (int)((i * MAX_PACKET_LEN) + sent));
	}	
	
	printf("\n\nCompleted\n");
	close(fd);
	return 1;
}

/**
 * checkServerResponse
 *
 * Checks whether or not server received valid data
 * for file.  If it gets an DATA_INVALID error it returns false,
 * returns true on A.  If it gets another error (something server
 * may have sent that might have been ignore) we close connection.
 *
 * Args:
 * int sock - socket descriptor for connection
 *
 * Returns:
 * Bool of whether or not server has valid data received from us. 
 */
static bool checkServerResponse(int sock) {
	// Check for whether the server got all the data properly.
	uint8_t wait;
	int checkRet;
	checkRet = recv(sock, &wait, sizeof(wait), 0);
	if (checkRet == -1) {
		fprintf(getLog(), "ERROR: Recv failed: %s\n", strerror(errno));
		closeProgram(true, false);
	}
	if (wait == 'E') {
		// We need to know error code
		checkRet = recv(sock, &wait, sizeof(wait), 0);
		if (checkRet == -1) {
			fprintf(getLog(), "ERROR: Recv failed: %s\n", strerror(errno));
			closeProgram(true, false);
		}
		if (wait == DATA_INVALID) {
			return false;	
		} else {
			// symptoms of something more serious.  Disconnect
			fprintf(getLog(), "ERROR: Server quit with error no: %d\n", wait);
			closeProgram(true, false);
		}
	} else if (wait == 'A') {
		return true;
	}
	return false; // to keep compiler happy.
}



int main (int argc, char * argv[]){
	// 1 because the program name is always considered an argument.
	if (argc <= 1) {
		fprintf(stdout, USAGE);
		exit(1);
	}

	//Grab the command line options and initialize the log
	struct commandLine * opts = getOptions(argc, argv);
	initLog(CLIENT_LOG);
	fprintf(getLog(), "INFO: Command line opts were: %s %s %s\n", opts->address, opts->ports, opts->padPath);

	//Create an addrinfo struct for the server we are connecting to
	struct addrinfo * serverInfo = buildAddrInfo(opts->address, opts->ports);

	//Flavour text
	fprintf(stdout, "\n\n");
	fprintf(stdout, "Initiating file transfer... \n");
	fprintf(stdout, "Connecting using the Legendary File Transfer Protocol\n");
	
	
	//Attempts to grab a new socket
	int sock = getSocket(serverInfo);
	int check = connectTo(sock, serverInfo);
	if (check == -1){
		fprintf(getLog(), "ERROR: Cannot connect to server!\n");
		closeProgram(true, false);
	}
	
	fprintf(stdout, "Connected to remote host!\n");
	
	// Compute string representation of the pad digest.
	fprintf(stdout, "Calculating pad digest\n");
	uint8_t digest[MD5_DIGEST_BYTES];	
	getMd5DigestFromFile(opts->padPath, digest, getFileSizeFromFilename(opts->padPath));
	char digestStr[MD5_STRING_LENGTH];
	convertMd5ToString(digestStr, digest);

	fprintf(stdout, "Initiating secure file transfer...\n");

	//For each file in argv attempt to send it
	for(int i = argc - opts->fileNum; i < argc; i++){
		fprintf(stdout, "\nSending: %s\n", argv[i]);

		//Send a file and check for failure. Also check the MD5 checksum
		int check = sendFile(opts->address, opts->ports, argv[i], opts->padPath, digestStr, sock);
		if (check == -1) fprintf(stdout, "File was a directory!\n");
		else if (!checkServerResponse(sock)) {
			fprintf(getLog(), "WARNING: Server did not receive correct data.  Trying again...\n");
			i--; 
		}
	}
	//End the connection and free memory
	uint8_t d = 'D';
	sendAll(sock, &d, sizeof(d));

	free(opts);

	closeProgram(false, false);
}
