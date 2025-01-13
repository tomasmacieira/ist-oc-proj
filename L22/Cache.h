#ifndef CACHE_H
#define CACHE_H

/****************  Constants & Masks ***************/
#define offset_mask   0b0000000000111111            // offset bits
#define l1_idx_mask   0b0011111111000000            // L1 index bits
#define l2_idx_mask   0b0111111111000000            // L2 index bits
#define l22_idx_mask  0b0011111111000000            // L22 index bits, given the sets, we require one less bit for idx (8 bits instead of 9)
#define tag_mask      0b1000000000000000            // tag bits
#define first_idx_bit 0b0100000000000000            // Used to restore the most significant idx bit on L2 cache

#define DRAM_SIZE     (1024 * BLOCK_SIZE)           // in bytes, 1024 * 16 * 4 = 65535 = 2^16, 16 bits for the address
                                                    // 16 - 6 for offset - 9 bits for idx = 1 bits for tag

#define WORD_SIZE     4                             // in bytes, i.e 32 bit words
#define BLOCK_SIZE    (16 * WORD_SIZE)              // in bytes, 6 bits offset  16*4 = 2⁶ 

#define L1_SIZE       (256 * BLOCK_SIZE)            // in bytes
#define L1_LINENO     (L1_SIZE / BLOCK_SIZE)        //(256 * 16 * 24) / 16 * 24 = 256 = 2⁸ 8 bits for index

#define L2_SIZE       (512 * BLOCK_SIZE)            // in bytes
#define L2_LINENO     (L2_SIZE / BLOCK_SIZE)        // (512 * 16 * 24) / 16 * 24 = 512 = 2^9 9 bits for index
#define L2_SETNO      (L2_LINENO / 2)               // For the 2 way associative L2cache

#define MODE_READ  1
#define MODE_WRITE 0

#define DRAM_READ_TIME 100
#define DRAM_WRITE_TIME 50
#define L2_READ_TIME 10
#define L2_WRITE_TIME 5
#define L1_READ_TIME 1
#define L1_WRITE_TIME 1

#endif
