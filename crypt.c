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
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

#include "file.h"
#include "crypt.h"
#include "digest.h" 
#include "log.h"

#define EXTENSION ".dat"
#define MD5_STRING_LEN_NO_TERM 32
#define DIRECTORY_OF_OTP "otp/"
#define MAP_FILENAME "offset.txt"
#define MD5_LENGTH_STRING "32"

// Not implemented as function, so we do not have dynamically allocate the strings
// in a dynamically list of dynamically allocated DigestMap!
#define FILENAME_LEN 41 

/**
 * DigestMap
 *
 * Data structure to function like a map
 * where the digest string representation is the
 * key, and using that we can find file size and offset..  */
typedef struct _DigestMap {
	long long int offset;
	char digest[MD5_STRING_LENGTH];
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
	if (map == NULL || map_count == 0) {
		return -1;
	}
	for (int i=0; i < map_count; i++) {
		if (strcmp(digest, map[i]->digest) == 0) {
			return i;
		}
	}
	return -1;
}

/**
 * findFilename
 *
 * Computes filename based off digest.
 *
 * Args:
 * char * filename - pointer to memory of FILENAME_LEN bytes to write to
 * char * digestStr - string representation of digest.  Set NULL if if using digestRaw
 * uint8_t * digestRaw - binary representation of digest. Set NULL if provide digestStr
 */
static void findFilename(char * filename, char * digestStr, uint8_t * digestRaw) {
	// Only necessary if given raw digest
	memset(filename, 0, FILENAME_LEN);
	char digest[MD5_STRING_LENGTH];
	if (digestStr == NULL || digestRaw != NULL) {
		convertMd5ToString(digest, digestRaw);
	}

	strncat(filename, DIRECTORY_OF_OTP, FILENAME_LEN);
	strncat(filename, (digestStr == NULL ? digest : digestStr), FILENAME_LEN);
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
		fprintf(getLog(), "ERROR: Could not open offset file for writing: %s\n", strerror(errno));
		closeProgram(true, true);
	}

	// We don't save file size info incase file size changes.
	for (int i = 0; i < map_count; i++) { 
		fprintf(f, "%s:%lli\n", map[i]->digest, map[i]->offset);
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
			fprintf(getLog(), "ERROR: Could not allocate room for digest map\n");
			closeProgram(true, true);
		}
	} else {
		// We make a new pointer as things can go weird when it fails and pointer 
		// to original is assigned.
		DigestMap ** new  = realloc(map, sizeof(DigestMap *) * map_count);
		if (new == NULL) {
			fprintf(getLog(), "ERROR: Could not allocate room for digest map\n");
			closeProgram(true, true);
		}
		map = new;
	}

	map[map_count] = malloc(sizeof(DigestMap));
	if (map[map_count] == NULL) {
		fprintf(getLog(), "ERROR: Could not allocate room for digest map\n");
		closeProgram(true, true);
	}

	memcpy(map[map_count]->digest, digest, MD5_STRING_LENGTH);
	map[map_count]->offset = 0;

	map_count++;
}

/**
 * changeMapOffset *
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

/**
 * getOtp
 * 
 * Loads a particular OTP specified by digest.  
 *
 * Do not provide digest as a string representation.. give the
 * binary digest of it.
 *
 * Args:
 * char * digest - string representation of digest
 * uint8_t * otp - pointer to memory for segment of OTP specified by offset, and has size len.
 * long long int offset - offset in file to load from
 * long long int len - the amount of data to retrieve starting from offset
 */
static void getOtp(char * digest, uint8_t * otp, long long int offset, long long int len) {
	char filename[FILENAME_LEN];
	findFilename(filename, digest, NULL);

	FILE * f = fopen(filename, "rb");
	int fd = fileno(f);
	if (f == NULL) {
		fprintf(getLog(), "ERROR: Could not locate otp to use\n");
		closeProgram(true, true);
	}

	if (lseek(fd, offset, SEEK_SET) == -1) {
		fprintf(getLog(), "ERROR: File seek error: %s\n", strerror(errno));
		closeProgram(true, true);
	}
	if (fread(otp, sizeof(uint8_t), len, f) == 0) { 
		fprintf(getLog(), "ERROR: Failed to load OTP: %s\n", strerror(errno));
		closeProgram(true, true);
	}
	fclose(f);
}

void clientCrypt(uint8_t * data, int data_pos, char * filename, long long int offset, long long int len) {
	uint8_t otp[len];
	FILE * f = fopen(filename, "rb");
	int fd = fileno(f);
	if (lseek(fd, offset, SEEK_SET) == -1) {
		perror("lseek");		
		exit(1);
	}
	if (fread(otp, sizeof(uint8_t), len, f) == 0) {
		fprintf(getLog(), "ERROR: Failed to load OTP: %s\n", strerror(errno));
		closeProgram(true, false);
	}
	for (int i=0; i<len; i++) {
		data[i + data_pos] = data[i + data_pos] ^ otp[i];
	}
	fclose(f);
}

void getOffsetAndSize(char * digest, long long int * offset, long long int * size) {
	int i = findPosition(digest);
	if (i != -1) {
		*offset = map[i]->offset;
	} else {
		*offset = 0;
	}

	char filename[FILENAME_LEN];
	memset(filename, 0, FILENAME_LEN);
	findFilename(filename, digest, NULL);

	FILE * f = fopen(filename, "r");
	if (f == NULL) {
		fprintf(getLog(), "ERROR: Could not load specified OTP: %s\n", strerror(errno));
		*size = 0; // otp not found
	} else {
		int fd = fileno(f);
		*size = getFileSize(fd);
		fclose(f);
	}
}

void serverCrypt(uint8_t * data, int data_pos, char * digest, long long int  offset, long long int len) {
	uint8_t * otp = malloc(len);
	if (otp == NULL) {
		fprintf(getLog(), "ERROR: Could not allocate room for OTP.\n");
		closeProgram(true, true);
	}
	getOtp(digest, otp, offset, len);
	for (int i=0; i<len; i++) {
		data[i + data_pos] = data[i + data_pos] ^ otp[i];
	}
	free(otp);
}

void setOffset(char * digest, long long int offset) {
	// Ref for Mutex from Dr. Nicholas M. Boer's CMPT 360 Lock Lecture Slides
	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock (&lock);

	int position = findPosition(digest);
	if (position == -1) {
		// doesn't exist in map... add it.
		addToMap(digest);
		changeMapOffset(findPosition(digest), offset);
	} else {
		changeMapOffset(position, offset);
	}
	writeMap();
	pthread_mutex_unlock (&lock);
}



void loadOffsets() {
	FILE * f = fopen(MAP_FILENAME, "r");
	if (f == NULL) {
		// no file.. get out of here.
		return;
	}

	char digest[MD5_STRING_LENGTH];
	long long int offset;
	while (fscanf(f, "%" MD5_LENGTH_STRING "s:%lli\n", digest, &offset) == 2) {

		// Check if the file still exists!
		char filename[FILENAME_LEN];
		findFilename(filename, digest, NULL);
		FILE * otp = fopen(filename, "r");
		if (otp == NULL) {
			continue; 
		}
		setOffset(digest, offset);
	}
	fclose(f);
}

