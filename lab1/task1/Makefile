CC = gcc
CFLAGS=-Wall -Wextra
TARGET=SimpleCache
TARGETL1=L1Cache

all:
	$(CC) $(CFLAGS) SimpleProgram.c SimpleCache.c -o $(TARGET)

l1:
	$(CC) $(CFLAGS) L1Cache.c -o $(TARGET) 

clean:
	rm $(TARGET)