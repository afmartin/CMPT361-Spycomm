/*

AWESOME HEADER BLOCK

*/

#include "netCode.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int recvAll (int sock, int amount, uint8_t * dest){
	
	uint8_t * pos = dest;
	
	while (pos - dest < amount){
		int received = recv(sock, pos, amount - (pos - dest), 0);
		if (received == -1){
			fprintf(stderr, "Error recieving data\n");
			perror("recv: ");
			return -1;
		}
		pos += received;
	}
	return amount;
}


int send_string(int sd, uint8_t * buf){
  
  size_t queued = MAXLEN;
  ssize_t sent;
  while (queued > 0){
    sent = send(sd, buf, queued, 0);
    if (sent == -1)
      return FALSE;
    queued -= sent;
    buf += sent;
  }
  return TRUE;
}

int connectTo (int sock, struct addrinfo * info){
  
  int returnCode = connect(sock, (struct sockaddr *) info->ai_addr, info->ai_addrlen);
  
  if (returnCode != 0){
    fprintf(stderr, "Failed to connect!/n");
    return sock = -1;
  }
	
	return sock;
}

//Inspired by code seen in Beej's Guide to Network Programming
//http://beej.us/guide/bgnet/
int sendAll(int sock, uint8_t * buffer, int len){
	
	int sent = 0;
	int left = len;
	int ret;
	
	while (sent < len){
		ret = send(sock, buffer + sent, left, 0);
		if (ret == -1){ return -1; }
		sent += ret;
		left -= ret;
	}
	
	return sent;
}

int acceptCon(int socket) {
  /* Handles client requests 
     will be updated to support Port Knocking */

  struct sockaddr_storage clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  int cd;

  cd = accept(socket, (struct sockaddr *) &clientAddr, &clientAddrLen);
  if (cd == -1) {
    fprintf(stderr, "Error accepting connections\n");
    exit(1);
  }
  return cd;
}

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