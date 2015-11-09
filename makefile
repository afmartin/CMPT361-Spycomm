CFLAGS= -std=c99 -g -o 
CC= gcc

.PHONY: clean all


all: zip compress

clean:
	$(RM) *.o


zip: zip.c
	$(CC) $(CFLAGS) $@ $^

compress: compress.c 
	$(CC) $(CFLAGS) $@ $^
