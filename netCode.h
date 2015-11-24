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
#define MAX_FILES_LEN 512
#define MD5LEN 16

#define TRUE 1
#define FALSE 0
#define MAXLEN 512
#define MAX_FILE_NAME 64
#define MAX_FILE_LENGTH_AS_STRING 21 

#define T_TYPE 'T'
#define F_TYPE 'F'
#define D_TYPE 'D'

int recvAll (int sock, int amount, uint8_t * dest);

//takes a socket and an addrinfo struct and attempts to connect to a remote host
int connectTo (int sock, struct addrinfo * info);

// function to send a string over a socket descriptor
int send_string(int sd, uint8_t * buf);



//Inspired by code seen in Beej's Guide to Network Programming
//http://beej.us/guide/bgnet/

//Given a uint8_t array sends the array until all the data is sent,
//or something goes wrong. Takes a socket, the buffer, the length of buffer
//and returns a boolean for success, and sets the len variable to the 
//amount of bytes actually sent
int sendAll(int sock, uint8_t * buffer, int len);


//Accepts a new connection on the given socket
int acceptCon(int socket);

//Build and addrinfo struct linkedlist given an address and port
struct addrinfo * buildAddrInfo (char * address, char * port);

#endif //ends NETCODE_H
