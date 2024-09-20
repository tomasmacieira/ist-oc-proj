#include "SimpleCache.h"

int main() {

  // set seed for random number generator
  srand(0);

  int clock1, value;

  for(int n = 1; n <= DRAM_SIZE/4; n*=WORD_SIZE) {

    resetTime();
    initCache();

    printf("\nNumber of words: %d\n", (n-1)/WORD_SIZE + 1);
    
    for(int i = 0; i < n; i+=WORD_SIZE) {
      write(i, (unsigned char *)(&i));
      clock1 = getTime();
      printf("Write; Address %d; Value %d; Time %d\n", i, i, clock1);
    }

    for(int i = 0; i < n; i+=WORD_SIZE) {
      read(i, (unsigned char *)(&value));
      clock1 = getTime();
      printf("Read; Address %d; Value %d; Time %d\n", i, value, clock1);
    }  

  }

  printf("\nRandom accesses\n");

  // Do random accesses to the cache
  for(int i = 0; i < 100; i++) {
    int address = rand() % (DRAM_SIZE/4);
    address = address - address % WORD_SIZE;
    int mode = rand() % 2;
    if (mode == MODE_READ) {
      read(address, (unsigned char *)(&value));
      clock1 = getTime();
      printf("Read; Address %d; Value %d; Time %d\n", address, value, clock1);
    }
    else {
      write(address, (unsigned char *)(&address));
      clock1 = getTime();
      printf("Write; Address %d; Value %d; Time %d\n", address, address, clock1);
    }
  }
  
  return 0;
}