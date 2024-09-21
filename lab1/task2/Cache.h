#ifndef CACHE_H
#define CACHE_H

#define DRAM_SIZE (1024 * BLOCK_SIZE)               // in bytes, 1024 * 16 * 4 = 65535 = 2¹6, 16 bits for the address
                                                    // 16 - 6 for offset - 9 bits for idx = 1 bits for tag

#define WORD_SIZE 4                                 // in bytes, i.e 32 bit words
#define BLOCK_SIZE (16 * WORD_SIZE)                 // in bytes, 6 bits offset  16*4 = 2⁶ 

#define L1_SIZE (256 * BLOCK_SIZE)                  // in bytes
#define L1_LINENO   (L1_SIZE / BLOCK_SIZE)          //(256 * 16 * 24) / 16 * 24 = 256 = 2⁸ 8 bits for index

#define L2_SIZE (512 * BLOCK_SIZE)                  // in bytes
#define L2_LINENO   (L2_SIZE / BLOCK_SIZE)          //(512 * 16 * 24) / 16 * 24 = 512 = 2^9 9 bits for index

#define MODE_READ 1
#define MODE_WRITE 0

#define DRAM_READ_TIME 100
#define DRAM_WRITE_TIME 50
#define L2_READ_TIME 10
#define L2_WRITE_TIME 5
#define L1_READ_TIME 1
#define L1_WRITE_TIME 1

#endif
