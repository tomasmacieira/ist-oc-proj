#include "L2Cache.h"

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

  if (!Line->Valid || Line->Tag != Tag) {               // if block not present - miss
    accessL2(address, TempBlock, MODE_READ);      // search for block in L2

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

  uint32_t index, Tag, MemAddress, offset;
  uint8_t TempBlock[BLOCK_SIZE];

  /* init cache */
  if (Cache2.init == 0) {
    for (index = 0; index < L2_LINENO; index++) { // Go through every line in the block
        Cache2.lines[index].Valid = 0;
    }
    Cache2.init = 1;
  }

  offset = address & offset_mask;                 // Get offset
  index = (address & l2_idx_mask) >> 6;           // Get index
  Tag = address >> 15;                            // Get tag

  CacheLine *Line = &Cache2.lines[index];

  MemAddress = address >> 6;                      // Remove offset from the address
  MemAddress = MemAddress << 6;                   // Restore removed bits with 0's

  /* access Cache*/

  if (!Line->Valid || Line->Tag != Tag) {                           // if block not present - miss
    accessDRAM(MemAddress, TempBlock, MODE_READ);                   // get new block from DRAM

    if ((Line->Valid) && (Line->Dirty)) {                           // line has dirty block
      MemAddress = (Line->Tag << 15) | (index << 6);                // get address of the block in memory
      accessDRAM(MemAddress, &(L2Cache[(index * BLOCK_SIZE) + offset]), MODE_WRITE);    // then write back old block
    }

    memcpy(&(L2Cache[(index * BLOCK_SIZE) + offset]), TempBlock, BLOCK_SIZE); // copy new block to cache line
    Line->Valid = 1;
    Line->Tag = Tag;
    Line->Dirty = 0;
  } // if miss, then replaced with the correct block

  if (mode == MODE_READ) {    // read data from cache line
    memcpy(data, &(L2Cache[(index * BLOCK_SIZE) + offset]), WORD_SIZE);     // Write back to L1
    time += L2_READ_TIME;
  }

  if (mode == MODE_WRITE) { // write data from cache line
    memcpy(&(L2Cache[(index * BLOCK_SIZE) + offset]), data, WORD_SIZE);                     // Write back to L1
    time += L2_WRITE_TIME;
    Line->Dirty = 1;
  }
}

void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}
