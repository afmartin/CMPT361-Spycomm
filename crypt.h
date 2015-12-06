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

#include <stdbool.h>
#include <stdint.h> // for uint8_t

/**
 * setOTPInUse
 *
 * Sets the status of the OTP to in use if it is already
 * not in use.
 *
 * NOTE: Only call this if you are sure OTP exists.
 * If this is called and OTP doesn't exist, it will
 * be added to the map.
 *
 * Args:
 * char * digest - the digest of the OTP
 *
 * Returns:
 * Boolean value of whether or not we can use
 */
bool setOTPInUse(char * digest);

/**
 * setOTPDone
 *
 * Sets the status of the OTP to not in use.
 *
 * Args:
 * char * digest - the digest of the OTP
 */
void setOTPDone(char * digest);

/** 
 * getOffsetAndSize
 *
 * Finds offset and the size of an OTP specified by digest.
 *
 * char * digest - the digest of OTP (string form)
 * long long int * offset - a pointer to write offset of otp to
 * long long int * size - a pointer to write size of otp to.  If zero otp does not exist
 */
void getOffsetAndSize(char * digest, long long int * offset, long long int * size); 

/**
 * servercrypt
 *
 * Either encrypts or decrypts an array of bytes by a one time pad.
 *
 * NOTE: Function will alter data provided, if you want to keep
 * original, make a duplicate before calling this function.
 *
 * Args: 
 * uint8_t * data - data to alter
 * int data_pos - is there an offset in the data to encrypt
 * char * digest - the digest of OTP 
 * long long int offset - position currently in one time pad (offset)
 * long long int len - for how many bytes do we "crypt"
 */
void serverCrypt(uint8_t * data, int data_pos, char * digest, long long int  offset, long long int len);

/**
 * clinetCrypt
 *
 * Either encrypts or decrypts an array of bytes by a one time pad.
 *
 * NOTE: Function will alter data provided, if you want to keep
 * original, make a duplicate before calling this function.
 *
 * Args: 
 * uint8_t * data - data to alter
 * int data_pos - is there an offset in the data to encrypt
 * char * filename - file name of OTP. 
 * long long int offset - position currently in one time pad (offset)
 * long long int len - for how many bytes do we "crypt"
 */
void clientCrypt(uint8_t * data, int data_pos, char * filename, long long int offset, long long int len);

/**
 * setOffset
 *
 * Updates the offset/digest map of where we are in offset.  Will write to
 * file.
 *
 * Args:
 * char * digest - the digest
 * long long int offset - the new offset
 */
void setOffset(char * digest, long long int offset);

/**
 * loadOffsets
 *
 * Loads offset map from file.  
 *
 * MUST BE CALLED ON SERVER INITIALIZATION!!!
 */
void loadOffsets();

#endif /* _CRYPT_H */
