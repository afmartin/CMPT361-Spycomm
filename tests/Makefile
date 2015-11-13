#############################################################
#CMPT 361 - Assignment 3
#Group 4: Nick, John, Alex, Kevin
#Date: November 7th, 2015
#Filename: Makefile
#Description: A Makefile for Tests
#############################################################
CC = gcc
CFLAGS = -D_POSIX_C_SOURCE=200809L -Wall -pedantic -std=c99

.PHONY: all clean

all: digest_test 
digest_test: digest_test.o ../digest.o ../digest/md5.o ../digest/common.o
	$(CC) $(LDFLAGS) -o $@ $^ 
../digest/md5.o: ../digest/md5.c ../digest/md5.h
../digest/common.o: ../digest/common.c ../digest/common.h 
../digest.o: ../digest.c ../digest.h
digest_test.o: digest_test.c 

clean:
	$(RM) *.o digest_test 
