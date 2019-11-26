#pragma once

#include "queue.h"
#include "bitmatrix.h"

#pragma pack(push, 1)
struct qr_ecb {
    const unsigned char ecc_blocks;
    const unsigned char data_words;
};

struct qr_ecblocks {
    const unsigned char ec_codewords_per_block;
    const unsigned char qr_ecb_size;
    const struct qr_ecb ecb[2];
};

struct qr_version_block {
    const unsigned char alignment_pattern_num;
    const unsigned char alignment_pattern_centers[7];
    const struct qr_ecblocks ecblocks[4];
};
extern const struct qr_version_block qr_version_blocks[40];
#pragma pack(pop)

struct qr_data_block {
    unsigned int data_codewords;
    unsigned int length;
    unsigned char codewords[0];
};

extern unsigned int qr_code_ncodewords(const unsigned int version);

extern struct bitmatrix *qr_build_function_pattern(const unsigned int version);

extern struct circular_queue *qr_data_block_split(const unsigned int version, const unsigned int error_correction_level,
    const unsigned char *rawcodes);
