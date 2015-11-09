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

spycommd: server.o crypt.o compress.o user.o
	$(CC) server.o crypt.o compress.o user.o -o spycommd

spycomm: client.o crypt.o compress.o user.o
	$(CC) client.o crypt.o compress.o user.o -o spycomm

server.o: server.c 
client.o: client.c
compress.o: compress.c compress.h
user.o: user.c user.h

clean:
	$(RM) spycommd spycomm *.o
