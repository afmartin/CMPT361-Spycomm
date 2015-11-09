/*
  John Robbins
  CMPT 361
  November 8, 2015

  zip.c includes functions that encode an array of bytes into a
  64bit encoded string as well as decode back to their original
  values.
*/
#ifndef ZIP_H_
#define ZIP_H_

#define B64_NUM_SYMBOLS 64

// encodes an array into 64-bit encoded value 
char * encode(uint8_t * data, int length);
// decodes a 64-bit encoded string into an array of bytes
uint8_t * decode(char * coded);

#endif  // end ZIP_H_ definition 
