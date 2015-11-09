
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "zip.h"
#include <stdio.h>


static char b64_table[B64_NUM_SYMBOLS];
static char initialized = false;
// code taken from class notes
static void init_table(){
  
  int i;
  for (i = 0; i < B64_NUM_SYMBOLS; i++){
    i < 26 ? b64_table[i] = (char)(i - 0) + 'A':
      i < 52 ? b64_table[i] = (char)(i - 26) + 'a':
      i < 62 ? b64_table[i] = (char)(i - 52) + '0':
      i == 62 ? '+' : '/';
  }

  initialized = true;
}

// code taken from class notes
char * encode(uint8_t * data, int length){
  
  int padding = length % 3;
  // length of returned b64 string 
  int codedLen = 4 * ( (padding+length) / 3) + 1;
  uint8_t * padded;
  const int coder = 63; // 6 bits of 1's
  
  
  // init table if not already initiialized
  if (!initialized)
    init_table();

  // pad the data
  padded = malloc(length + padding);
  memset(padded, 0, length);
  memcpy(padded, data, length);
  
  // b64 string 
  char * encoded = malloc(sizeof(char) * codedLen);
  // create pointer to first element of the string
  char * ptr = encoded;

  for (int i = 0; i < (length); i+=3){
    
    // create a block of three items shifted into 
    // a 32 bit block -- only 24 bits used
    uint32_t block = ((uint32_t)padded[i] << 16 |
		      (uint32_t)padded[i+1] << 8 |
		      (uint32_t)padded[i+2]);
    
    for (int ii = 18; ii >= 0; ii-=6){
      // shift items -> AND with 63 to get 6 bits
      uint32_t idx = (block >> ii) & coder;
      *ptr++ = b64_table[idx];
    }
  }
  // account for the padding 
  for (int i = 0; i < padding; i++)
    *ptr++ = '=';
  *ptr = '\0';  // terminate string 
  
  free(padded); // free padded 
  return encoded;
}

// gets numeric value of symbol in table 
static int getIndex(char symbol){
  
  if (!initialized) // init table in case not already
    init_table;
  
  // iterate thorugh table looking for symbol
  for (int i = 0; i < B64_NUM_SYMBOLS; i++)
    if (symbol == b64_table[i]) return i;  // return index
  return -1;
}
  
uint8_t * decode(char * coded){
  
  int len = strlen(coded); 
  // get length of string before it was coded
  int realLen = (len / 4) * 3;
  uint8_t * data;
  const int decoder = 255; // 8 bits of 1's

  // allocate data on the heap
  data = malloc(sizeof(uint8_t) * realLen);
  // set pointer to beggining of data
  uint8_t *ptr = data;
  for (int i = 0; i < len; i+= 4){
    
    // create a 32 bit block by adding bits of 6 to th block
    // only use 24 bits
    uint32_t block = ((uint32_t)getIndex(coded[i]) << 18 |
		      (uint32_t)getIndex(coded[i+1]) << 12 |
		      (uint32_t)getIndex(coded[i+2]) << 6 |
		      (uint32_t)getIndex(coded[i+3]));
    
    //shift over by multiple of 8 -> AND with 255 to get decoded byte
    *ptr++= (block >> 16) & decoder;
    *ptr++ = (block >> 8) & decoder;
    *ptr++ = block & decoder;
  }
  
  return data;
}  

int main(void){
  
  uint8_t message[] = {23, 54, 76, 77, 255, 5, 55, 44, 33, 22, 11, 0};
  int length = 12;
  char * coded = encode(message, length);
  uint8_t * decoded = decode(coded);
  
  printf("%s\n", coded);
  for (int i = 0; i < length; i++)
    printf("%d\n", decoded[i]);
  
}
