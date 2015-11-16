#############################################################
#CMPT 361 - Assignment 3
#Group 4: Nick, John, Alex, Kevin
#Date: November 7th, 2015
#Filename: Makefile
#Description:
#############################################################

CC = gcc
CFLAGS = -D_POSIX_C_SOURCE=200809L -Wall -pedantic -std=c99
LDLIBS = -lpthread

.PHONY: all clean

all: spycommd spycomm

spycommd: server.o crypt.o compress.o user.o file.o netCode.o
	$(CC) $(LDLIBS) $^ -o $@

spycomm: client.o crypt.o compress.o user.o file.o netCode.o
	$(CC) $(LDLIBS) $^ -o $@

server.o: server.c 
client.o: client.c
compress.o: compress.c compress.h
user.o: user.c user.h
file.o: file.c file.h
netCode.o: netCode.c netCode.h

clean:
	$(RM) spycommd spycomm *.o

serv: server.c file.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

