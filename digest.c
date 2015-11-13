/*
#################################################################################
CMPT 361 - Assignment 3                                                         
Group 4: Nick, John, Alex, Kevin
November 6th, 2015
Filename: digest.c 
Description: Functions for computing MD5 digests
#################################################################################
*/

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "digest.h"

bool compareMd5Digest(uint8_t * a, uint8_t * b) {
	for (int i=0; i<MD5_DIGEST_BYTES; i++) {
		if (a[i] != b[i]) {
			return false;
		}
	}	
	return true;
}

void getMd5Digest(uint8_t * data, size_t len, uint8_t * digest) {
	memset(digest, 0, MD5_DIGEST_BYTES);
	struct md5CTX md;
	md5Start(&md);
	int code = md5Add(&md, data, len);
	if (code != 0) {
		fprintf(stderr, "ERROR: Data too big for MD5 digest to be computed.");
	}
	md5End(&md, digest);
}

void convertMd5ToString(char * string, uint8_t * digest) {
	memset(string, 0, MD5_STRING_LENGTH);
	for (int i=0; i<MD5_DIGEST_BYTES; i++) {
		snprintf(string + (i * 2), MD5_STRING_LENGTH - (i*2), "%02x", digest[i]);
	}
}


