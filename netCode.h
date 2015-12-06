/*

##########################################################################
CMPT 361 - Assignment 3
Group 4: Nick, John, Alex, Kevin
December 4th, 2015
Filename: netCode.h
Description: header file for netCode.c. Contains references to functions
and several macros used.
##########################################################################

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
#define MAX_FILE_NAME 128
#define MAX_FILE_LENGTH_AS_STRING 21 
#define AUTHENTICATION_LENGTH 16

#define A_TYPE 'A'
#define T_TYPE 'T'
#define F_TYPE 'F'
#define D_TYPE 'D'

// Error Number Definitions
// Check RFC for details.
#define NO_ROOM 1 // close connection
#define PAD_INVALID 2 // close connection
#define DATA_INVALID 3 // wait for data to be resent
#define AUTH_NO_MATCH 4  // close connection
#define CON_HANG 5 // close connection
#define UNSPEC_ERROR 6 // close connection
#define IN_USE 7 // close connection

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
int acceptCon(int socket, struct sockaddr_storage *clientAddr);

//Build and addrinfo struct linkedlist given an address and port
struct addrinfo * buildAddrInfo (char * address, char * port);

#endif //ends NETCODE_H
