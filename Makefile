#############################################################
#CMPT 361 - Assignment 3
#Group 4: Nick, John, Alex, Kevin
#Date: November 7th, 2015
#Filename: Makefile
#Description: Makefile for both spycommd and spycomm
#############################################################
# Note: -FILE_OFF_SET_BITS=64 makes sure that fstats uses a 64 bit value to represent a file size
# even on a 32 bit system.
CC = gcc
CFLAGS = -D_POSIX_C_SOURCE=200809L -D_FILE_OFFSET_BITS=64 -g -Wall -pedantic -std=c99
LDLIBS = -lpthread -lm -lncurses

.PHONY: all clean

all: spycommd spycomm

spycommd: server.o crypt.o compress.o file.o netCode.o digest.o digest/md5.o digest/common.o screen.o log.o
	$(CC)  $^ -o $@ $(LDLIBS)

spycomm: client.o crypt.o compress.o file.o netCode.o digest.o digest/md5.o digest/common.o log.o
	$(CC)  $^ -o $@ $(LDLIBS)

server.o: server.c server.h file.h netCode.h crypt.h digest.h screen.h log.h
client.o: client.c file.h netCode.h digest.h crypt.h log.h
file.o: file.c file.h log.h
netCode.o: netCode.c netCode.h log.h
screen.o: screen.c screen.h
crypt.o: crypt.c crypt.h file.h digest.h log.h
digest.o: digest.c digest.h file.h log.h digest/md5.h
digest/md5.o: digest/md5.c digest/md5.h digest/common.h
digest/common.o: digest/common.c digest/common.h
log.o: log.c log.h

clean:
	$(RM) spycommd spycomm *.o digest/*.o
