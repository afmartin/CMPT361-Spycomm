#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compress.h"

frequencyTable * findFrequencies(char * string){
  
  int len = strlen(string);
  int nodeFound = 0;
  frequencyTable * table, *iter;
  table  = (frequencyTable *)malloc(sizeof(frequencyTable));
  iter = table;
  
  table->symbol = string[0];
  table->freq = 1;
  
  for (int i = 1; i < len; i++){
    while (iter->next){
      if (string[i] == iter->symbol){
	iter->freq++;
	nodeFound = 1;
	break;
      }
      else iter = iter->next;
    }
    if (!nodeFound){
      iter->next = (frequencyTable *)malloc(sizeof(frequencyTable));
      iter->next->symbol = string[i];
      iter->next->freq = 1;
    }
    nodeFound = 0;
    iter = table;
  }
  return table;
}

void traverseFreqTable(frequencyTable ** table){
  
  frequencyTable * iter;
  for (iter = *table; iter; iter = iter->next)
    printf("%c = %d", iter->symbol, iter->freq);
  
}

int main(void){
  
  frequencyTable ** table;
  char * string = "hello world";
  *table = findFrequencies(string);
  traverseFreqTable(table);
  
  return 0;
}

//node * initTree(frequencyTable ** frequencies){
