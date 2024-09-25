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
    for (index = 0; index < L1_LINENO; index++) {         // Go through every line in the block
        Cache1.lines[index].Valid = 0;
    }
    Cache1.init = 1;
  }

  offset = address & offset_mask;                         // Get offset (bits 0-5)
  index = (address & l1_idx_mask) >> 6;                   // Get index  (bits 6-13) and remove the offset bits
  Tag = address >> 14;                                    // Get tag and remove the 8 idx bits + 6 offset bits

  CacheLine *Line = &Cache1.lines[index];

  MemAddress = address >> 6;                              // Remove offset from the address
  MemAddress = MemAddress << 6;                           // Restore removed bits with 0's

  /* access Cache*/

  if (!Line->Valid || Line->Tag != Tag) {                 // if block not present - miss
    accessL2(address, TempBlock, MODE_READ);              // search for block in L2

    if ((Line->Valid) && (Line->Dirty)) {                                               // line has dirty block
      MemAddress = (Line->Tag << 14) | (index << 6);                                    // get address of the block in memory
      accessL2(MemAddress, &(L1Cache[(index * BLOCK_SIZE) + offset]), MODE_WRITE);      // then write back old block
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

  uint32_t lineTwoIndex, index, Tag, MemAddress, offset;
  uint8_t TempBlock[BLOCK_SIZE];

  /* init cache */
  if (Cache2.init == 0) {
    for (index = 0; index < L2_SETNO; index++) {            // Go through every line in the block
        Cache2.sets[index].lineOne.Valid = 0;               // Setting up valid bits on both lines of the set
        Cache2.sets[index].lineTwo.Valid = 0;               // Default LRU is set for the first line
        Cache2.sets[index].LRU = 0;
    }
    Cache2.init = 1;
  }

  offset = address & offset_mask;                           // Get offset (bits 0-5)
  index = (address & l22_idx_mask) >> 6;                    // Get index  (bits 6-13) and remove the 6 offset bits
  lineTwoIndex = index || first_idx_bit;                    // Used to access the second line in set
  Tag = address >> 14;                                      // Get tag and remove the 6 offset bits + 8 idx bits

  Set *Set = &Cache2.sets[index];                           // Set selected by the index
  CacheLine *lineOne = &Cache2.sets[index].lineOne;         // Reference for the first line in the selected set
  CacheLine *lineTwo = &Cache2.sets[index].lineTwo;         // Reference for the second line in the selected set

  uint8_t lineOneTag = Set->lineOne.Tag;                    // lineOne tag used to find the corresponding requested tag
  int lruLine = 0;                                          // Target address is located in either the first or second line
  if (lineOneTag != Tag) {lruLine = 1;}                     // Swap to lineTwo

  MemAddress = address >> 6;                                // Remove offset from the address
  MemAddress = MemAddress << 6;                             // Restore removed bits with 0's

  /* access Cache*/

  // CASE 1: Both lines are invalid or lineTwo tag doest not correspond
  if ((!lineOne->Valid && !lineTwo->Valid) || (!lineOne->Valid && lineTwo->Tag != Tag)) {
    accessDRAM(MemAddress, TempBlock, MODE_READ);                                             // get new block from DRAM
    memcpy(&(L2Cache[(index * BLOCK_SIZE) + offset]), TempBlock, BLOCK_SIZE);                 // copy new block to cache line

    lineOne->Valid = 1;
    lineOne->Tag = Tag;
    lineOne->Dirty = 0;
    Set->LRU = 1;                 // Line two (1) was the least recently used 
  } 
  // CASE 2: lineTwo is invalid and lineOne tag does not correspond
  else if ((!lineTwo->Valid && lineOne->Tag != Tag)) {
    accessDRAM(MemAddress, TempBlock, MODE_READ);                                            // get new block from DRAM
    memcpy(&(L2Cache[(lineTwoIndex * BLOCK_SIZE) + offset]), TempBlock, BLOCK_SIZE);         // copy new block to cache line

    lineTwo->Valid = 1;
    lineTwo->Tag = Tag;
    lineTwo->Dirty = 0;
    Set->LRU = 0;                 // Line one (0) was the least recently used
  }
  // CASE 3: Both lines are valid and have different tags from the requested one 
  else if (lineOne->Tag != Tag && lineTwo->Tag != Tag) {                        
      if (Set->LRU == 0) {                                                                   // if lineOne (0) is LRU -> Replace it
          accessDRAM(MemAddress, TempBlock, MODE_READ);                                      // get new block from DRAM

          if (lineOne->Dirty) {
            MemAddress = (lineOne->Tag << 14) | (index << 6);                                // get address of the block in memory
            accessDRAM(MemAddress, &(L2Cache[(index * BLOCK_SIZE) + offset]), MODE_WRITE);   // then write back old block
          }
          
          memcpy(&(L2Cache[(index * BLOCK_SIZE) + offset]), TempBlock, BLOCK_SIZE);          // copy new block to cache line
          lineOne->Valid = 1;
          lineOne->Tag = Tag;
          lineOne->Dirty = 0;
          Set->LRU = 1;                                                                       // Used lineOne (0), lineTwo (1) is the LRU

      } 
      else {                                                                                 // if lineTwo (1) is LRU -> Replace it
        accessDRAM(MemAddress, TempBlock, MODE_READ);                                        // get new block from DRAM

          if (lineTwo->Dirty) {
            MemAddress = (lineTwo->Tag << 14) | (index << 6);                                       // get address of the block in memory
            accessDRAM(MemAddress, &(L2Cache[(lineTwoIndex * BLOCK_SIZE) + offset]), MODE_WRITE);   // then write back old block
          }
          memcpy(&(L2Cache[(index * BLOCK_SIZE) + offset]), TempBlock, BLOCK_SIZE);                 // copy new block to cache line
          lineTwo->Valid = 1;
          lineTwo->Tag = Tag;
          lineTwo->Dirty = 0;
          Set->LRU = 0;                                                                       // Used lineTwo (1), lineOne (0) is the LRU
      }
  }

  // if miss, then replaced with the correct block
  if (mode == MODE_READ) {                                                        // read data from cache line
    if (lineOne->Tag == Tag) {                                                           // lineOne is the one asked by L1 Cache
      memcpy(data, &(L2Cache[(index * BLOCK_SIZE) + offset]), WORD_SIZE);         // Write back to L1 (lineOne index)
      Set->LRU = 1;                                                                       // Used lineOne (0), lineTwo (1) is the LRU
    } 
    else {                                                                        // lineTwo is the line asked by L1 Cache
      memcpy(data, &(L2Cache[(lineTwoIndex * BLOCK_SIZE) + offset]), WORD_SIZE);  // Write back to L1 (lineTwo index)
      Set->LRU = 0;                                                                       // Used lineTwo (1), lineOne (0) is the LRU
    }
    time += L2_READ_TIME;
  }

  if (mode == MODE_WRITE) {                                                        // write data from cache line
    if (lineOne->Tag == Tag) {                                                            // lineOne is the one asked by L1 Cache
      memcpy(&(L2Cache[(index * BLOCK_SIZE) + offset]), data, WORD_SIZE);          // Write back to L1 (lineOne index)
      Set->LRU = 1;                                                                       // Used lineOne (0), lineTwo (1) is the LRU
      lineOne->Dirty = 1;
    } 
    else {                                                                         // lineTwo is the one asked by L1 Cache
      memcpy(&(L2Cache[(lineTwoIndex * BLOCK_SIZE) + offset]), data, WORD_SIZE);   // Write back to L1 (lineTwo index)
      Set->LRU = 0;                                                                       // Used lineTwo (1), lineOne (0) is the LRU
      lineTwo->Dirty = 1;
    }
    time += L2_WRITE_TIME;
  }
}

void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}
