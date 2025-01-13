CC = gcc
CFLAGS = -Wall -Wextra
TARGET = L22Cache

all:
	$(CC) $(CFLAGS) tester.c L22Cache.c -o $(TARGET)

test:
	$(CC) $(CFLAGS) tester.c L22Cache.c -o $(TARGET)
	./$(TARGET) > myout.txt
	diff myout.txt ../tests/results_L2_2W.txt > diff.txt

clean:
	rm -f $(TARGET) *.txt
