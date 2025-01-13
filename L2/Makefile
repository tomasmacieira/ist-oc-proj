CC = gcc
CFLAGS = -Wall -Wextra
TARGET = L2Cache

all:
	$(CC) $(CFLAGS) tester.c L2Cache.c -o $(TARGET)

test:
	$(CC) $(CFLAGS) tester.c L2Cache.c -o $(TARGET)
	./$(TARGET) > myout.txt
	diff myout.txt ../tests/results_L2_1W.txt > diff.txt

clean:
	rm -f $(TARGET) *.txt
