/*
#############################################################
CMPT 361 - Assignment 3
Group 4: Nick, John, Alex, Kevin
November 20th, 2015
Filename: server.h
Description:
#############################################################
*/

#ifndef _SERVER_H
#define _SERVER_H

typedef struct _fileInfo fileInfo;

//This function prints out the usage for the spycommd exec.
void printUsage(char* name);

//Grab a socket and return it. Exits on failure.
int getSocket(char* port);

int getPadOffset(int padID);

int initFileTransfer(int cd, fileInfo *info);

void* worker(void * arg);

void inputHandler(int s);

#endif /* end _SERVER_H if */
