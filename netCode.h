/*

AWESOME HEADER BLOCK

*/

#ifndef NETCODE_H
#define NETCODE_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


#include <stdint.h>

#define IPV6_ADDRLEN 46
#define MAX_PORTS_LEN 32
#define MAX_PATH_LEN 64
#define MD5LEN 16

#define TRUE 1
#define FALSE 0
#define MAXLEN 512
#define MAX_FILE_NAME 64
#define MAX_MD5LEN 16
#define MAX_FILE_LENGTH_AS_STRING 10

#define T_TYPE 'T'
#define F_TYPE 'F'

void recvAll (int sock, int amount, uint8_t * dest);

//takes a socket and an addrinfo struct and attempts to connect to a remote host
int connectTo (int sock, struct addrinfo * info);

// function to send a string over a socket descriptor
int send_string(int sd, uint8_t * buf);

//Given a uint8_t array sends the array until all the data is sent,
//or something goes wrong. Takes a socket, the buffer, the length of buffer
//and returns a boolean for success, and sets the len variable to the 
//amount of bytes actually sent
int sendAll(int sock, uint8_t * buffer, int  * len);

int acceptCon(int socket);

//Build and addrinfo struct linkedlist given an address and port
struct addrinfo * buildAddrInfo (char * address, char * port);

#endif //ends NETCODE_H