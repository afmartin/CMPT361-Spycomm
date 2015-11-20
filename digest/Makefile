#############################################################
#CMPT 361 - Assignment 3
#Group 4: Nick, John, Alex, Kevin
#Date: November 7th, 2015
#Filename: Makefile
#Description:
#############################################################

CC = gcc
CFLAGS = -D_POSIX_C_SOURCE=200809L -Wall -pedantic -std=c99

.PHONY: all clean

all: md5 

md5: common.o md5.o
common.o: common.c common.h
md5.o: md5.c md5.h

clean:
	$(RM) md5 *.o 
