#ifndef SIMPLECACHE_H
#define SIMPLECACHE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Cache.h"

void resetTime();

uint32_t getTime();

/****************  Constants & Masks ***************/
long int offset_mask = 0b0000000000111111;    // offset bits
long int l1_idx_mask = 0b0011111111000000;    // L1 index bits
long int l2_idx_mask = 0b0111111111000000;    // L2 index bits
long int tag_mask    = 0b1000000000000000;    // tag bits

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t, uint8_t *, uint32_t);

/*********************** Cache *************************/

void initCache();
void accessL1(uint32_t, uint8_t *, uint32_t);
void accessL2(uint32_t, uint8_t *, uint32_t, uint8_t *);

typedef struct CacheLine {
  uint8_t Valid;
  uint8_t Dirty;
  uint32_t Tag;
} CacheLine;

typedef struct Cache {
  uint32_t init;
  CacheLine lines[L1_LINENO];
} Cache;

/*********************** Interfaces *************************/

void read(uint32_t, uint8_t *);

void write(uint32_t, uint8_t *);

#endif
