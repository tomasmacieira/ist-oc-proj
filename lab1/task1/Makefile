CC = gcc
CFLAGS = -Wall -Wextra
TARGET = L1Cache

all:
	$(CC) $(CFLAGS) SimpleProgram1.c L1Cache.c -o $(TARGET)

test:
	$(CC) $(CFLAGS) SimpleProgram1.c L1Cache.c -o $(TARGET)
	./$(TARGET) > myout.txt
	diff myout.txt ../tests/results_L1.txt > diff.txt

clean:
	rm -f $(TARGET) *.txt
