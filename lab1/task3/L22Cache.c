#include "L22Cache.h"

uint8_t L1Cache[L1_SIZE];
uint8_t L2Cache[L2_SIZE];
uint8_t DRAM[DRAM_SIZE];
uint32_t time;
Cache Cache1;
CacheL2 Cache2;

/**************** Time Manipulation ***************/
void resetTime() { time = 0; }

uint32_t getTime() { return time; }

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t address, uint8_t *data, uint32_t mode) {

  if (address >= DRAM_SIZE - WORD_SIZE + 1)
    exit(-1);

  if (mode == MODE_READ) {
    memcpy(data, &(DRAM[address]), BLOCK_SIZE);
    time += DRAM_READ_TIME;
  }

  if (mode == MODE_WRITE) {
    memcpy(&(DRAM[address]), data, BLOCK_SIZE);
    time += DRAM_WRITE_TIME;
  }
}

/*********************** L1 cache *************************/

void initCache() { 
  Cache1.init = 0;
  Cache2.init = 0; 
}

void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t index, Tag, MemAddress, offset;
  uint8_t TempBlock[BLOCK_SIZE];

  /* init cache */
  if (Cache1.init == 0) {
    for (index = 0; index < L1_LINENO; index++) { // Go through every line in the block
        Cache1.lines[index].Valid = 0;
    }
    Cache1.init = 1;
  }

  offset = address & offset_mask;                 // Get offset
  index = (address & l1_idx_mask) >> 6;           // Get index
  Tag = address >> 14;                            // Get tag

  CacheLine *Line = &Cache1.lines[index];

  MemAddress = address >> 6;                      // Remove offset from the address
  MemAddress = MemAddress << 6;                   // Restore removed bits with 0's

  /* access Cache*/

  if (!Line->Valid || Line->Tag != Tag) {          // if block not present - miss
    accessL2(address, TempBlock, MODE_READ);       // search for block in L2

    if ((Line->Valid) && (Line->Dirty)) {                                               // line has dirty block
      MemAddress = (Line->Tag << 14) | (index << 6);                                    // get address of the block in memory
      accessL2(MemAddress, &(L1Cache[(index * BLOCK_SIZE) + offset]), MODE_WRITE);     // then write back old block
    }

    memcpy(&(L1Cache[(index * BLOCK_SIZE) + offset]), TempBlock, BLOCK_SIZE); // copy new block to cache line
    Line->Valid = 1;
    Line->Tag = Tag;
    Line->Dirty = 0;
  } // if miss, then replaced with the correct block

  if (mode == MODE_READ) {    // read data from cache line
    memcpy(data, &(L1Cache[(index * BLOCK_SIZE) + offset]), WORD_SIZE);
    time += L1_READ_TIME;
  }

  if (mode == MODE_WRITE) { // write data from cache line
    memcpy(&(L1Cache[(index * BLOCK_SIZE) + offset]), data, WORD_SIZE);
    time += L1_WRITE_TIME;
    Line->Dirty = 1;
  }
}

void accessL2(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t newIndex, index, Tag, MemAddress, offset;
  uint8_t TempBlock[BLOCK_SIZE];

  /* init cache */
  if (Cache2.init == 0) {
    for (index = 0; index < L2_SETNO; index++) { // Go through every line in the block
        Cache2.sets[index].lineOne.Valid = 0;
        Cache2.sets[index].lineTwo.Valid = 0;
        Cache2.sets[index].LRU = 0;
    }
    Cache2.init = 1;
  }

  offset = address & offset_mask;                 // Get offset
  index = (address & l22_idx_mask) >> 6;           // Get index
  Tag = address >> 14;                            // Get tag

  CacheLine *lineOne = &Cache2.sets[index].lineOne;
  CacheLine *lineTwo = &Cache2.sets[index].lineTwo;
  CacheLine *lruLine;
  Set *Set = &Cache2.sets[index];

  MemAddress = address >> 6;                      // Remove offset from the address
  MemAddress = MemAddress << 6;                   // Restore removed bits with 0's

  /* access Cache*/

  if ((!lineOne->Valid && !lineTwo->Valid) || (!lineOne->Valid && lineTwo->Tag != Tag)) {
    accessDRAM(MemAddress, TempBlock, MODE_READ);                                            // get new block from DRAM
    memcpy(&(L2Cache[(index * BLOCK_SIZE) + offset]), TempBlock, BLOCK_SIZE);                // copy new block to cache line
    lineOne->Valid = 1;
    lineOne->Tag = Tag;
    lineOne->Dirty = 0;
    Set->LRU = 1;       // Line two (1) was the least recently used 
  } else if ((!lineTwo->Valid && lineOne->Tag != Tag)) {
    newIndex = index || first_idx_bit;                                                // Used to access the second line in set
    memcpy(&(L2Cache[(newIndex * BLOCK_SIZE) + offset]), TempBlock, BLOCK_SIZE);
    lineTwo->Valid = 1;
    lineTwo->Tag = Tag;
    lineTwo->Dirty = 0;
    Set->LRU = 0;     // Line one (0) was the least recently used
  } else if (lineOne->Tag != Tag && lineTwo->Tag != Tag) {                        // Both are valid and have different tags
      if (Set->LRU == 0) {          // Replace the first line
          accessDRAM(MemAddress, TempBlock, MODE_READ);                                            // get new block from DRAM
          if (lineOne->Dirty) {
            MemAddress = (lineOne->Tag << 14) | (index << 6);                // get address of the block in memory
            accessDRAM(MemAddress, &(L2Cache[(index * BLOCK_SIZE) + offset]), MODE_WRITE);    // then write back old block
          }
          memcpy(&(L2Cache[(index * BLOCK_SIZE) + offset]), TempBlock, BLOCK_SIZE);
          lineOne->Valid = 1;
          lineOne->Tag = Tag;
          lineOne->Dirty = 0;
      } else {
        accessDRAM(MemAddress, TempBlock, MODE_READ);                                            // get new block from DRAM
          if (lineTwo->Dirty) {
            MemAddress = (lineTwo->Tag << 14) | (index << 6);                // get address of the block in memory
            newIndex = index || first_idx_bit;
            accessDRAM(MemAddress, &(L2Cache[(newIndex * BLOCK_SIZE) + offset]), MODE_WRITE);    // then write back old block
          }
          memcpy(&(L2Cache[(index * BLOCK_SIZE) + offset]), TempBlock, BLOCK_SIZE);
          lineTwo->Valid = 1;
          lineTwo->Tag = Tag;
          lineTwo->Dirty = 0;
      }
  }
  /*
  if ((!lineOne->Valid && lineTwo->Tag != Tag) || (!lineTwo->Valid && lineOne->Tag != Tag) ||
      (lineOne->Tag != Tag && lineTwo->Tag != Tag) || (!lineOne->Valid && !lineTwo->Valid)) {  // if block not present - miss
    accessDRAM(MemAddress, TempBlock, MODE_READ);                   // get new block from DRA

    if ((lruLine->Valid) && (lruLine->Dirty)) {                           // line has dirty block
      MemAddress = (lruLine->Tag << 14) | (index << 6);                // get address of the block in memory
      newIndex = index || ((Tag << 14) && tag_mask); 
      accessDRAM(MemAddress, &(L2Cache[(newIndex * BLOCK_SIZE) + offset]), MODE_WRITE);    // then write back old block
    }

    memcpy(&(L2Cache[(newIndex * BLOCK_SIZE) + offset]), TempBlock, BLOCK_SIZE); // copy new block to cache line
    lruLine->Valid = 1;
    lruLine->Tag = Tag;
    lruLine->Dirty = 0;
  } // if miss, then replaced with the correct block*/

  if (mode == MODE_READ) {    // read data from cache line
    memcpy(data, &(L2Cache[(newIndex * BLOCK_SIZE) + offset]), WORD_SIZE);     // Write back to L1
    time += L2_READ_TIME;
  }

  if (mode == MODE_WRITE) { // write data from cache line
    memcpy(&(L2Cache[(newIndex * BLOCK_SIZE) + offset]), data, WORD_SIZE);                     // Write back to L1
    time += L2_WRITE_TIME;
    lruLine->Dirty = 1;
  }
}

void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}
