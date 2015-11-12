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

#define OPTSTRING "hc:p:o:f:"

//Maybe this could be in user.c?
#define DEFAULT_PORT "36115"

struct commandLine {
	char * address;
	char * ports;
	char * padPath;
	char * filePath;
};

void freeCommandLine (struct commandLine * cmd){
	free(cmd->address);
	free(cmd->ports);
	free(cmd->padPath);
	free(cmd->filePath);
	free(cmd);
}

struct commandLine * getOptions (int argc, char * argv[]){
	struct commandLine * options = malloc(sizeof(struct commandLine));
	options->address = malloc(IPV6_ADDRLEN);
	options->ports = malloc(MAX_PORTS_LEN);
	options->padPath = malloc(MAX_PATH_LEN);
	options->filePath = malloc(MAX_PATH_LEN);
		
	if (options->address == NULL || options->ports == NULL || options->padPath == NULL || options->filePath == NULL){
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

struct addrinfo * buildAddrInfo (char * address, char * port){
	
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(struct addrinfo));
	
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AI_V4MAPPED;
	
	int returnCode = getaddrinfo(address, port, &hints, &res);
	
	if (returnCode != 0){
		fprintf(stderr, "Getaddrinfo failure!\n");
		exit(1);
	}
	
	else{
		return res;
	}
	
}

int getSocket (struct addrinfo * info){
	
	struct addrinfo * iter;
	
	int sock = -1;
	
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

int connectTo (int sock, struct addrinfo * info){
	
	int returnCode = connect(sock, (struct sockaddr *) info->ai_addr, info->ai_addrlen);
	
	if (returnCode != 0){
		fprintf(stderr, "Failed to connect!/n");
		return sock;
	}
	
	return sock;
}

int main (int argc, char * argv[]){

	struct commandLine * opts = getOptions(argc, argv);
	
	printf("Command line opts were: %s %s %s\n", opts->address, opts->ports, opts->padPath);
	
	freeCommandLine(opts);

	return 1;
}