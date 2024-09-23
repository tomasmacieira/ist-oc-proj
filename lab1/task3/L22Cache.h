#ifndef L22CACHE_H
#define L22CACHE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Cache.h"

void resetTime();

uint32_t getTime();

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t, uint8_t *, uint32_t);

/*********************** Cache *************************/

void initCache();
void accessL1(uint32_t, uint8_t *, uint32_t);
void accessL2(uint32_t, uint8_t *, uint32_t);

typedef struct CacheLine {
  uint8_t Valid;
  uint8_t Dirty;
  uint32_t Tag;
} CacheLine;

typedef struct Set {
  CacheLine lineOne;
  CacheLine lineTwo;
  uint8_t LRU;                      // LRU = 0 -> least recently used line is lineOne
} Set;                              // LRU = 1 -> least recently used line is lineTwo

typedef struct Cache {
  uint32_t init;
  CacheLine lines[L1_LINENO];
} Cache;

typedef struct CacheL2 {
  uint32_t init;
  Set sets[L2_SETNO];
} CacheL2;

/*********************** Interfaces *************************/

void read(uint32_t, uint8_t *);

void write(uint32_t, uint8_t *);

#endif
