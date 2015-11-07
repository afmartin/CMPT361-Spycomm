#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "zip.h"
#include <stdio.h>

//#define std c99
#define B64_NUM_SYMBOLS  64

static char b64_table[B64_NUM_SYMBOLS];
static char initialized = false;

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

char * encode(uint8_t * data, int length){
  
  //int i;
  int padding = length % 3;
  int codedLen = 4 * ( (padding+length) / 3) + 1;
  uint8_t * padded;
  const int coder = 63;
  char previous;
  
  if (!initialized)
    init_table();

  // pad the data
  padded = malloc(length + padding);
  memset(padded, 0, length);
  memcpy(padded, data, length);
  
  char * encoded = malloc(sizeof(char) * codedLen);
  char * ptr = encoded;

  for (int i = 0; i < (length); i+=3){
    
    uint32_t block = ((uint32_t)padded[i] << 16 |
		      (uint32_t)padded[i+1] << 8 |
		      (uint32_t)padded[i+2]);
    
    for (int ii = 18; ii >= 0; ii-=6){
      uint32_t idx = (block >> ii) & coder;
      if (previous == 'A' && b64_table[idx] == 'A'){
	//ptr--;
	break;
      }
      previous = *ptr++ = b64_table[idx];
    }
  }
  
  for (int i = 0; i < padding; i++)
    *ptr++ = '=';
  *ptr = '\0';
  
  free(padded);
  return encoded;
}
  
uint8_t * decode(char * coded){
  
  int len = strlen(coded);
  int realLen = (len / 4) * 3;
  uint8_t * data;
  const int decoder = 255;

  data = malloc(sizeof(uint8_t) * realLen);
  uint8_t *ptr = data;
  for (int i = 0; i < len; i+= 4){
    
    uint8_t block = ((uint32_t)coded[i] << 18 |
		     (uint32_t)coded[i+1] << 12 |
		     (uint32_t)coded[i+2] << 6 |
		     (uint32_t)coded[i+3]);
    
    *ptr++= (block >> 16) & decoder;
    *ptr++ = (block >> 8) & decoder;
    *ptr++ = block & decoder;
  }
  
  return data;
}  

int main(void){
  
  char * message = "1234";
  int length = 4;
  char * coded = encode(message, length);
  char * decoded = decode(coded);
  
  printf("%s\n", coded);
  for (int i = 0; i < length; i++)
    printf("%d\n", decoded[i]);
 
}
