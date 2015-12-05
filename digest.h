/*
#################################################################################
CMPT 361 - Assignment 3                                                         
Group 4: Nick, John, Alex, Kevin
November 6th, 2015
Filename: digest.h 
Description: Functions for computing MD5 digests
#################################################################################
*/
#ifndef _DIGEST_H_
#define _DIGEST_H_

#include <stdbool.h>
#include <stdint.h>
#include "digest/md5.h"

#define MD5_STRING_LENGTH 33

/**
 * getMd5DigestFromFile
 *
 * Computes MD5 digest from a given file. 
 *
 *  char * filename - Filename of file.  Does not check if it exists. 
 *  uint8_t * digest - MUST HAVE LENGTH MD5_DIGEST_BYTES
 *					 - where the digest is written to
 *	long long int filesize - The what part of the file do we get sum for 
 *						   - Added because a file may change in between calculating filesize & md5
 */
void getMd5DigestFromFile(char * filename, uint8_t * digest, long long int filesize);

/**
 * getMd5Digest
 *
 * Computes MD5 digest from given bytes and length.
 *
 *  uint8_t * data - Array of bytes to use for MD5 digest
 *  size_t len - length of data
 *  uint8_t * digest - MUST HAVE LENGTH MD5_DIGEST_BYTES
 *					 - where the digest is written to
 */
void getMd5Digest(uint8_t * data, size_t len, uint8_t * digest);

/**
 * compareMd5Digest
 *
 * Compares two byte arrays with length MD_DIGEST_BYTES.
 *
 * uint8_t * a - first digest
 * uint8_t * b - second digest
 *
 * Returns true if a == b, false if a != b
 */
bool compareMd5Digest(uint8_t * a, uint8_t * b);

/**
 * convertMd5ToString
 *
 * Converts md5 digest to string representation.
 *
 * char * string - pointer to allocated space for digest's string representation
 * uint8_t * digest - Binary version of digest
 */
void convertMd5ToString(char * string, uint8_t * digest);

#endif /* _DIGEST_H_ */
