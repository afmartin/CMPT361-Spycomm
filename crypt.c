/*
#################################################################################
CMPT 361 - Assignment 3                                                         
Group 4: Nick, John, Alex, Kevin
November 6th, 2015
Filename: crypt.c 
Description: Functions for dealing with OTPs and Offsets.
#################################################################################
*/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "file.h"
#include "crypt.h"
#include "digest.h" 

#define EXTENSION ".dat"
#define MD5_STRING_LEN_NO_TERM 32
#define DIRECTORY_OF_OTP "otp/"
#define MAP_FILENAME "offset.txt"
#define MD5_LENGTH_STRING "33"

// Not implemented as function, so we do not have dynamically allocate the strings
// in a dynamically list of dynamically allocated DigestMap!
#define FILENAME_LEN 41 

/**
 * DigestMap
 *
 * Data structure to function like a map
 * where the digest string representation is the
 * key, and the value is the offset.
 */
typedef struct _DigestMap {
    unsigned long offset;
    char digest[FILENAME_LEN];
} DigestMap;

static DigestMap ** map;
static int map_count = 0;

/**
 * findPosition
 *
 * Finds the position in the digest map, that
 * a the key is found.  
 *
 * Returns -1 if nothing found.
 *
 * Args:
 * char * digest - the string representation of digest.
 */
static int findPosition(char * digest) {
    for (int i=0; i < map_count; i++) {
        if (strcmp(digest, map[i]->digest)) {
            return i;
        }
    }
    return -1;
}

/**
 * getOffset
 *
 * Given a binary digest, find the current offset.
 *
 * Args:
 * uint8_t * digest - binary representation of digest 
 *
 * Returns:
 * unsigned long of the offset is found.
 */
static unsigned long getOffset(uint8_t * digest) {
    char digestStr[MD5_STRING_LENGTH];
    convertMd5ToString(digestStr, digest);
    int position = findPosition(digestStr);
    if (position == -1) {
        return 0;
    } else {
        return map[position]->offset;
    }
} 

/**
 * findFilename
 *
 * Computes filename based off digest.
 *
 * Args:
 * char * filename - pointer to memory of FILENAME_LEN bytes to write to
 * uint8_t * digest - binary representation of digest.
 */
static void findFilename(char * filename, uint8_t * digest) {
    char digestStr[MD5_STRING_LENGTH];
    convertMd5ToString(digestStr, digest);

    strncat(filename, DIRECTORY_OF_OTP, FILENAME_LEN);
    strncat(filename, digestStr, FILENAME_LEN);
    strncat(filename, EXTENSION, FILENAME_LEN);
}

/**
 * writeMap
 *
 * Writes the current state of the digest-offset map
 * to a file (will clobber it).
 */
static void writeMap() {
	FILE * f = fopen(MAP_FILENAME, "w"); 
	if (f == NULL) {
		fprintf(stderr, "Could not open offset file for writing.\n");
		exit(1);
	}

	for (int i = 0; i < map_count; i++) { 
		fprintf(f, "%s:%lu\n", map[i]->digest, map[i]->offset);
	}
	fclose(f);
}

/**
 * addToMap
 *
 * Adds a digest to a map (assumes that the digest
 * is not in the map already.. or will get duplicates).
 *
 * Args:
 * char * digest - string representation of the digest
 */
static void addToMap(char * digest) {
    // Any malloc error leads to program termination,
    // as if malloc fails something is seriously wrong.
    if (map_count == 0) {
        // Map is empty
        map = malloc(sizeof(DigestMap *));
        if (map == NULL) {
            fprintf(stderr, "ERROR: Could not allocate room for digest map");
            exit(1);
        }
    } else {
        // We make a new pointer as things can go weird when it fails and pointer 
        // to original is assigned.
        DigestMap ** new  = realloc(map, sizeof(DigestMap *) * map_count);
        if (new == NULL) {
            fprintf(stderr, "ERROR: Could not allocate room for digest map");
            exit(1);
        }
        map = new;
    }

    map[map_count] = malloc(sizeof(DigestMap));
    if (map[map_count] == NULL) {
        fprintf(stderr, "ERROR: Could not allocate room for digest map");
        exit(1);
    }

    memcpy(map[map_count]->digest, digest, MD5_STRING_LENGTH);
    map[map_count]->offset = 0;

    map_count++;
}

/**
 * changeMapOffset
 *
 * Updates an element of the map (given an integer position)
 * with the new offset.
 *
 * Args: 
 * int position - position in array of maps
 * unsigned long offset - current offset:q
 */
static void changeMapOffset(int position, unsigned long offset) {
    map[position]->offset = offset;
}

void setOffset(uint8_t * digest, unsigned long offset) {
    char digestStr[MD5_STRING_LENGTH];
    convertMd5ToString(digestStr, digest);

    int position = findPosition(digestStr);
    if (position == -1) {
        // doesn't exist in map... add it.
        addToMap(digestStr);
		changeMapOffset(findPosition(digestStr), offset);
    } else {
        changeMapOffset(position, offset);
    }
    writeMap();
}

void crypt(uint8_t * data, int data_pos, uint8_t * otp, int otp_pos, int len) {
    for (int i=0; i<len; i++) {
        data[i + data_pos] = data[i + data_pos] ^ otp[i + otp_pos];
    }
}

uint8_t * getOtp(uint8_t * digest, unsigned long * offset) {
    char filename[FILENAME_LEN];
    findFilename(filename, digest);
    
    FILE * f = fopen(filename, "r");
    if (f == NULL) {
        return NULL;  
    }

    uint8_t * otp = malloc(DIGEST_SIZE);

    if (getFileArray(f, DIGEST_SIZE, otp) == -1) {
        free(otp);
        return NULL;
    }
	fclose(f);

    *offset = getOffset(digest);
    return otp;
}

void readMap() {
	FILE * f = fopen(MAP_FILENAME, "r");
	if (f == NULL) {
		// no file.. get out of here.
		return;
	}

	char digest[MD5_STRING_LENGTH];
	unsigned long offset;
	while (fscanf(f, "%" MD5_LENGTH_STRING "s:%lu", digest, offset) == 2) {
		setOffset(digest, offset);
	}
	fclose(f);
}

