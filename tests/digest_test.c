/*
#################################################################################
CMPT 361 - Assignment 3                                                         
Group 4: Nick, John, Alex, Kevin
November 6th, 2015
Filename: digest_test.c 
Description: Test Digest Functions 
#################################################################################
*/

#include "../digest.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void testGetMd5Digest(uint8_t * data, size_t len, uint8_t * expected) {
	uint8_t * digest = malloc(sizeof(uint8_t) * MD5_DIGEST_BYTES);
	getMd5Digest(data, len, digest);
	if (compareMd5Digest(digest, expected)) {
		printf("Passed.  Expected:");
	} else {
		printf("Failed.  Expected: ");
	}
	char str[MD5_STRING_LENGTH];
	convertMd5ToString(str, expected);
	printf("%s", str);
	printf(" | Got: ");
	convertMd5ToString(str, digest);
	printf("%s", str);
	printf(".\n");
	free(digest);
}

int main() {
	uint8_t d1[] = {'A', 'b', 'C', 'd', 'E'};
	uint8_t d2[] = {'A', 'A', 'A', 'A','A', 'A', 'A', 'A', 'A', 'A', 'A','A', 'A', 'A', 'A', 'A', 'A', 'A','A', 'A', 'A', 'A', 'A', 'A', 'A','A', 'A', 'A', 'A'};
	uint8_t d3[] = {'H', 'e', 'l', 'l', 'o'};

	int len[] = {3, 29, 5};
uint8_t expected[][16] = { {'\x25','\xaa','\x3e','\xe1','\xc9','\x3c','\xad','\x3f','\x27','\x45','\x67','\x28','\x10','\x66','\xdc','\x18'}, {'\xcf','\x52','\x05','\xdc','\x20','\xfb','\x05','\x14','\x5e','\x6d','\x1f','\xa0','\x81','\x66','\xe9','\x4e'},
		{'\x8b','\x1a','\x99','\x53','\xc4','\x61','\x12','\x96','\xa8','\x27','\xab','\xf8','\xc4','\x78','\x04','\xd7'}
	};

	testGetMd5Digest(d1, len[0], expected[0]);
	testGetMd5Digest(d2, len[1], expected[1]);
	testGetMd5Digest(d3, len[2], expected[2]);
}
