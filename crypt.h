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
uint8_t * getOtp(uint8_t * digest, unsigned long * pos);
void setOffset(uint8_t digest, unsigned long offset);
void loadOffsets();

#endif /* _CRYPT_H */
