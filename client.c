/*

VERY INSIGHTFUL AND INORMATIVE COMMENT BLOCK GOES HERE

*/	

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define IPV6_ADDRLEN 46
#define MAX_PORTS_LEN 32
#define MAX_PATH_LEN 64

#define OPTSTRING "hc:p:o:"

//Maybe this could be in user.c?
#define DEFAULT_PORT "36115"

struct commandLine {
	char * address;
	char * ports;
	char * padPath;
};

void freeCommandLine (struct commandLine * cmd){
	free(cmd->address);
	free(cmd->ports);
	free(cmd->padPath);
	free(cmd);
}

struct commandLine * getOptions (int argc, char * argv[]){
	struct commandLine * options = malloc(sizeof(struct commandLine));
	options->address = malloc(IPV6_ADDRLEN);
	options->ports = malloc(MAX_PORTS_LEN);
	options->padPath = malloc(MAX_PATH_LEN);
		
	if (options->address == NULL || options->ports == NULL || options->padPath == NULL){
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
			default:
				printf("Usage: %s [-h] -c \"SERVERADDRESS\" -p \"PORTS_TO_KNOCK\" -o \"PATH_TO_OTP\"\n", argv[0]);
				exit(0);
				break;
		}
	}
	
	return options;
}


int main (int argc, char * argv[]){

	struct commandLine * opts = getOptions(argc, argv);
	
	printf("Command line opts were: %s %s %s\n", opts->address, opts->ports, opts->padPath);
	
	freeCommandLine(opts);

	return 1;
}