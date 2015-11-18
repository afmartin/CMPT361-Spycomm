/*
#################################################################################
CMPT 361 - Assignment 3                                                         
Group 4: Nick, John, Alex, Kevin
November 6th, 2015
Filename: crypt.h 
Description: Functions for dealing with OTPs and Offsets.
#################################################################################
*/
#ifndef _CRYPT_H_ 
#define _CRYPT_H_

#include <stdint.h> // for uint8_t

#define DIGEST_SIZE 10485760 // size in bytes of a one time pad

/**
 * crypt
 *
 * Either encrypts or decrypts an array of bytes by a one time pad.
 *
 * NOTE: Function will alter data provided, if you want to keep
 * original, make a duplicate before calling this function.
 *
 * Args: 
 * uint8_t * data - data to alter
 * int data_pos - is there an offset in the data to encrypt
 * uint8_t - the one time pad
 * int otp_pos - position currently in one time pad (offset)
 * int len - for how many bytes do we "crypt"
 */
void crypt(uint8_t * data, int data_pos, uint8_t * otp, int otp_pos, int len);

/**
 * getOtp
 * 
 * Loads a particular OTP specified by digest.  
 *
 * Do not provide digest as a string representation.. give the
 * binary digest of it.
 *
 * Args:
 * uint8_t * digest - Array of bytes to represent digest
 * unsigned long * offset - updates offset pointer to where offset is
 *
 * Returns:
 * uint8_t * to malloc'd (remember to free) of OTP.  Will be 10MB!
 */
uint8_t * getOtp(uint8_t * digest, unsigned long * offset);

/**
 * setOffset
 *
 * Updates the offset/digest map of where we are in offset.  Will write to
 * file.
 *
 * Args:
 * uint8_t * digest - Array of bytes to represent digest (not string representation)
 * unsigned long offset - the new offset
 */
void setOffset(uint8_t * digest, unsigned long offset);

/**
 * loadOffsets
 *
 * Loads offset map from file.  
 *
 * MUST BE CALLED ON SERVER INITIALIZATION!!!
 */
void loadOffsets();

#endif /* _CRYPT_H */
