#############################################################
#CMPT 361 - Assignment 3
#Group 4: Nick, John, Alex, Kevin
#Date: November 7th, 2015
#Filename: Makefile
#Description:
#############################################################

CC = gcc
# Note: -FILE_OFF_SET_BITS=64 makes sure that fstats uses a 64 bit value to represent a file size
# even on a 32 bit system.
CFLAGS = -D_POSIX_C_SOURCE=200809L -D_FILE_OFFSET_BITS=64 -g -Wall -pedantic -std=c99
LDLIBS = -lpthread -lm

.PHONY: all clean

all: spycommd spycomm

spycommd: server.o crypt.o compress.o user.o file.o netCode.o digest.o digest/md5.o digest/common.o
	$(CC)  $^ -o $@ $(LDLIBS)

spycomm: client.o crypt.o compress.o user.o file.o netCode.o digest.o digest/md5.o digest/common.o
	$(CC)  $^ -o $@ $(LDLIBS)

server.o: server.c 
client.o: client.c
compress.o: compress.c compress.h
user.o: user.c user.h
file.o: file.c file.h
netCode.o: netCode.c netCode.h
crypt.o: crypt.c crypt.h
digest.o: digest.c digest.h
digest/md5.o: digest/md5.c digest/md5.h
digest/common.o: digest/common.c digest/common.h

clean:
	$(RM) spycommd spycomm *.o digest/*.o


