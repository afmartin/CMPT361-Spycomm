/*

   VERY INSIGHTFUL AND INORMATIVE COMMENT BLOCK GOES HERE

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

#include "file.h"
#include "netCode.h"
#include "digest.h"
#include "crypt.h"
#include "log.h"

#define OPTSTRING "hc:p:o:"

#define USAGE "USAGE: %s [-h] -c \"SERVERADDRESS\" -p \"PORT\" -o \"PATH_TO_OTP\" filenames...\n", argv[0]

#define DONE   printf("Done!\n");

#define CLIENT_LOG "spycomm.txt"

//This is a struct to hold the command line strings

// TODO: Be advised that althougth the max file length may be
// 512 bytes, the user could reference files from subfolders and therefor
// the length should dynamically be determined.
struct commandLine {
	char address[IPV6_ADDRLEN];
	char ports[MAX_PORTS_LEN];
	char padPath[MAX_PATH_LEN];
	char filePath[MAX_FILES_LEN];
	int fileNum;
};

//Grabs the command line options and places them into their
//coresponding parts of the commandLine Struct
struct commandLine * getOptions (int argc, char * argv[]){
	struct commandLine * options = malloc(sizeof(struct commandLine));

	if (options == NULL){
		fprintf(getLog(), "ERROR: Memory allocation failed!\n");
		closeProgram(true, false);
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

	if(count <= 0){
		fprintf(stderr,"ERROR: No files listed!\n");
		fprintf(stdout, USAGE);
		exit(1);
	}

	memset(options->filePath, 0, MAX_FILES_LEN);

	int argvPos = optind;
	int filePathPos = 0;

	// TODO: Check if files actually exist.  Otherwise ignore them.
	while (count != 0){
		if (filePathPos + strlen(argv[argvPos]) + 1 > MAX_FILES_LEN){
			fprintf(stderr, "ERROR: Too Many files chosen!\n");
			exit(0);
		}
		strncpy(&(options->filePath[filePathPos]), argv[argvPos], MAX_PATH_LEN);
		filePathPos += strlen(argv[argvPos]) + 1;
		argvPos++;
		count--;
		options->fileNum++;
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

	freeaddrinfo(info);
	return sock;
}

//Parses error codes then exits
void printErrorThenExit(uint8_t errorCode){
	switch (errorCode){
		case NO_ROOM:
			fprintf(getLog(), "ERROR: The server indicates insufficient pad is remaining\n");
			closeProgram(true, false);
			break;
		case PAD_INVALID:
			fprintf(getLog(), "ERROR: The server indicates that an invlaid pad was used\n");
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
			fprintf(getLog(), "ERROR: The server indicates that unspecified error occured\n");
			closeProgram(true, false);
			break;
		default:
			fprintf(getLog(), "WARNING: UNKNOWN ERROR CODE RECIEVED\n");
			closeProgram(true, false);
			break;
	}
}

//Sends the initialization to the server given the socket, filename, length of file,
//MD5 padID, and returns true or false depending on whether it succeeds
long long int initiateFileTransfer(int sock, char * fileName, char * length, char * padPath){
	
	// Compute string representatino of digest.
	uint8_t digest[MD5_DIGEST_BYTES];	
	getMd5DigestFromFile(padPath, digest);
	char digestStr[MD5_STRING_LENGTH];
	convertMd5ToString(digestStr, digest);
	
	
	//Get the length of the initialization packet
	int initStringLen = sizeof(char) + strlen(fileName) + strlen(length) + strlen(digestStr) + 3;

	
	
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
		fprintf(getLog(), "ERROR: Recieve failed while waiting for an aknowledgement\n");
		closeProgram(true, false);
	}
	else if (wait[0] != 'T'){
		if (wait[0] == 'E'){
			printErrorThenExit(wait[1]);
		}
		else {
			fprintf(getLog(), "ERROR: Expected a 'T' or 'E' type, but recieved '%c'\n", wait[0]);
			printf("%s\n", wait);
			closeProgram(true, false);
		}
	}
	
	//Make sure we recieved all of the handshaking data
	if (checkRet != sizeof(wait)){
		int returnValue = recvAll(sock, sizeof(wait) - checkRet, wait + checkRet);
		if (returnValue == -1){
			fprintf(getLog(), "ERROR: Recieve failed while waiting for handshake\n");
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
		fprintf(getLog(), "ERROR: Recieve failed while waiting for an aknowledgement\n");
		closeProgram(true, false);
	}
	else if (wait[0] != 'A'){
		if (wait[0] == 'E'){
			printErrorThenExit(wait[1]);
		}
		else {
			fprintf(getLog(), "ERROR: Expected a 'T' or 'E' type, but recieved '%c'\n", wait[0]);
			closeProgram(true, false);
		}
	}
	printf("We are authenticated!\n");
	printf("Trasfering........\n\n");
	return offset + AUTHENTICATION_LENGTH;
}

void sendFile (char * address, char * port, char * fileName, char * padPath, int sock){

	//Get info about the file to send
	FILE * fp = fopen(fileName, "r");
	int fd = fileno(fp);
	long long int fileSize = getFileSize(fd);
	char fileLenAsString[MAX_FILE_LENGTH_AS_STRING];
	snprintf(fileLenAsString, MAX_FILE_LENGTH_AS_STRING, "%lli", fileSize);

	if(fileSize == 0){
		printf("File is empty!");
	}	
	
	//Sends the initialization data and recieved the offset to use
	long long int offset = initiateFileTransfer(sock, fileName, fileLenAsString, padPath);
	if (offset == -1){
		fprintf(getLog(), "Error handshaking: program terminating\n");
		closeProgram(true, false);
	}

	//Create the buffer for the Packet to be sent
	uint8_t buffer[MAX_PACKET_LEN + 1];
	memset(buffer, 0, MAX_PACKET_LEN + 1);

	int barWidth = 60;

	//Send out as many packets as there is data
	for (int i = 0; i <= fileSize / (MAX_PACKET_LEN); i++){
		buffer[0] = 'F';
		size_t read = fread(buffer + 1, 1, MAX_PACKET_LEN, fp);
		if (read == -1){
			fprintf(getLog(), "ERROR: An Error occured reading %s\n", fileName);
			closeProgram(true, false);
		}
		//		printf("%s\n", padPath);
		clientCrypt(buffer, 1, padPath, offset, MAX_PACKET_LEN);
		offset += read;

		int sent = sendAll(sock, buffer, read + 1);
		if (sent == -1){
			fprintf(getLog(), "ERROR: Failed to send: %s\n", strerror(errno));
			closeProgram(true, false);
		}

		int percent = (int)(ceil((float)((i + 1) * 100) / ceil((float)fileSize / (MAX_PACKET_LEN))));
		//printf("%2d%%  %d =  %d ' '\n", i / (fileSize / (MAX_PACKET_LEN)), ((percent * barWidth) / 100), (((100 - percent) * barWidth) / 100));

		printf("%2d%% [", percent);
		for (int j = 0; j < ((percent * barWidth) / 100); j++){
			printf("=");
		}
		for (int j = 0; j < (((100 - percent) * barWidth) / 100); j++){
			printf(" ");
		}
		printf("] %dB Sent         \r", (int)((i * MAX_PACKET_LEN) + sent));
	}
	printf("\nCompleted\n");
	close(fd);
	//freeaddrinfo(serverInfo);
}



int main (int argc, char * argv[]){
	// 1 because the program name is always considered an argument.
	if (argc <= 1) {
		fprintf(stdout, USAGE);
		exit(1);
	}

	struct commandLine * opts = getOptions(argc, argv);
	initLog(CLIENT_LOG);
	fprintf(getLog(), "INFO: Command line opts were: %s %s %s\n", opts->address, opts->ports, opts->padPath);

	struct addrinfo * serverInfo = buildAddrInfo(opts->address, opts->ports);

	printf("\n\n");
	printf("Initiating file transfer... \n");
	printf("Connecting using the Legendary File Transfer Protocol\n");
	//Attempts to grab a new socket
	int sock = getSocket(serverInfo);
	int check = connectTo(sock, serverInfo);
	if (check == -1){
		fprintf(getLog(), "ERROR: Cannot connect to server!\n");
		closeProgram(true, false);
	}
	printf("Connected to remote host!\n");
	printf("Initiating secure file transfer...\n");
	int pos = 0;
	for(int i = 0; i < opts->fileNum; i++){
		char fileToSend[MAX_PATH_LEN];
		strncpy(fileToSend, &(opts->filePath[pos]), MAX_PATH_LEN);

		printf("\nSending: %s\n", fileToSend);

		sendFile(opts->address, opts->ports, fileToSend, opts->padPath, sock);

		pos += strlen(fileToSend) + 1;
	}
	uint8_t d = 'D';
	sendAll(sock, &d, sizeof(d));

	free(opts);

	closeProgram(false, false);
}
