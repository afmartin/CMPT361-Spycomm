CFLAGS= -std=c99 -o
CC= gcc

.PHONY: clean all


all: zip

clean:
	$(RM) *.o


zip: zip.c
	$(CC) $(CFLAGS) $@ $^
