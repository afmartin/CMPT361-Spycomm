/*

AWESOME HEADER BLOCK

*/

#include "netCode.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>

void recvAll (int sock, int amount, uint8_t * dest){
	
	uint8_t temp;
	int pos = 0;
	
	while (pos < amount){
		int received = recv(sock, &temp, sizeof(uint8_t), 0);
		if (received == -1){
			fprintf(stderr, "Error recieving data\n");
		}
		*dest = temp;
		dest++;
		pos++;
	}
}