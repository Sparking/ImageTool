#include <stdint.h>
#include <stdlib.h>
#include "qr_version.h"
#include "port_memory.h"

const struct qr_version_block qr_version_blocks[40] = {
[0] = {.alignment_pattern_num = 0, .ecblocks = {
    [0] = {.ec_codewords_per_block = 7, .qr_ecb_size = 1, {{.ecc_blocks = 1, .data_words = 19}, {.ecc_blocks = 0, .data_words = 0}}},
    [1] = {.ec_codewords_per_block = 10, .qr_ecb_size = 1, {{.ecc_blocks = 1, .data_words = 16}, {.ecc_blocks = 0, .data_words = 0}}},
    [2] = {.ec_codewords_per_block = 13, .qr_ecb_size = 1, {{.ecc_blocks = 1, .data_words = 13}, {.ecc_blocks = 0, .data_words = 0}}},
    [3] = {.ec_codewords_per_block = 17, .qr_ecb_size = 1, {{.ecc_blocks = 1, .data_words = 9}, {.ecc_blocks = 0, .data_words = 0}}},
    }},
[1] = {.alignment_pattern_num = 2, .alignment_pattern_centers = {6, 18}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 10, .qr_ecb_size = 1, {{.ecc_blocks = 1, .data_words = 34}, {.ecc_blocks = 0, .data_words = 0}}},
    [1] = {.ec_codewords_per_block = 16, .qr_ecb_size = 1, {{.ecc_blocks = 1, .data_words = 28}, {.ecc_blocks = 0, .data_words = 0}}},
    [2] = {.ec_codewords_per_block = 22, .qr_ecb_size = 1, {{.ecc_blocks = 1, .data_words = 22}, {.ecc_blocks = 0, .data_words = 0}}},
    [3] = {.ec_codewords_per_block = 28, .qr_ecb_size = 1, {{.ecc_blocks = 1, .data_words = 16}, {.ecc_blocks = 0, .data_words = 0}}},
    }},
[2] = {.alignment_pattern_num = 2, .alignment_pattern_centers = {6, 22}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 15, .qr_ecb_size = 1, {{.ecc_blocks = 1, .data_words = 55}, {.ecc_blocks = 0, .data_words = 0}}},
    [1] = {.ec_codewords_per_block = 26, .qr_ecb_size = 1, {{.ecc_blocks = 1, .data_words = 44}, {.ecc_blocks = 0, .data_words = 0}}},
    [2] = {.ec_codewords_per_block = 18, .qr_ecb_size = 1, {{.ecc_blocks = 2, .data_words = 17}, {.ecc_blocks = 0, .data_words = 0}}},
    [3] = {.ec_codewords_per_block = 22, .qr_ecb_size = 1, {{.ecc_blocks = 2, .data_words = 13}, {.ecc_blocks = 0, .data_words = 0}}},
    }},
[3] = {.alignment_pattern_num = 2, .alignment_pattern_centers = {6, 26}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 20, .qr_ecb_size = 1, {{.ecc_blocks = 1, .data_words = 80}, {.ecc_blocks = 0, .data_words = 0}}},
    [1] = {.ec_codewords_per_block = 18, .qr_ecb_size = 1, {{.ecc_blocks = 2, .data_words = 32}, {.ecc_blocks = 0, .data_words = 0}}},
    [2] = {.ec_codewords_per_block = 26, .qr_ecb_size = 1, {{.ecc_blocks = 2, .data_words = 24}, {.ecc_blocks = 0, .data_words = 0}}},
    [3] = {.ec_codewords_per_block = 16, .qr_ecb_size = 1, {{.ecc_blocks = 4, .data_words = 9}, {.ecc_blocks = 0, .data_words = 0}}},
    }},
[4] = {.alignment_pattern_num = 2, .alignment_pattern_centers = {6, 30}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 26, .qr_ecb_size = 1, {{.ecc_blocks = 1, .data_words = 108}, {.ecc_blocks = 0, .data_words = 0}}},
    [1] = {.ec_codewords_per_block = 24, .qr_ecb_size = 1, {{.ecc_blocks = 2, .data_words = 43}, {.ecc_blocks = 0, .data_words = 0}}},
    [2] = {.ec_codewords_per_block = 18, .qr_ecb_size = 2, {{.ecc_blocks = 2, .data_words = 15}, {.ecc_blocks = 2, .data_words = 16}}},
    [3] = {.ec_codewords_per_block = 22, .qr_ecb_size = 2, {{.ecc_blocks = 2, .data_words = 11}, {.ecc_blocks = 2, .data_words = 12}}},
    }},
[5] = {.alignment_pattern_num = 2, .alignment_pattern_centers = {6, 34}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 18, .qr_ecb_size = 1, {{.ecc_blocks = 2, .data_words = 68}, {.ecc_blocks = 0, .data_words = 0}}},
    [1] = {.ec_codewords_per_block = 16, .qr_ecb_size = 1, {{.ecc_blocks = 4, .data_words = 27}, {.ecc_blocks = 0, .data_words = 0}}},
    [2] = {.ec_codewords_per_block = 24, .qr_ecb_size = 1, {{.ecc_blocks = 4, .data_words = 19}, {.ecc_blocks = 0, .data_words = 0}}},
    [3] = {.ec_codewords_per_block = 28, .qr_ecb_size = 1, {{.ecc_blocks = 4, .data_words = 15}, {.ecc_blocks = 0, .data_words = 0}}},
    }},
[6] = {.alignment_pattern_num = 3, .alignment_pattern_centers = {6, 22, 38}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 20, .qr_ecb_size = 1, {{.ecc_blocks = 2, .data_words = 78}, {.ecc_blocks = 0, .data_words = 0}}},
    [1] = {.ec_codewords_per_block = 18, .qr_ecb_size = 1, {{.ecc_blocks = 4, .data_words = 31}, {.ecc_blocks = 0, .data_words = 0}}},
    [2] = {.ec_codewords_per_block = 18, .qr_ecb_size = 2, {{.ecc_blocks = 2, .data_words = 14}, {.ecc_blocks = 4, .data_words = 15}}},
    [3] = {.ec_codewords_per_block = 26, .qr_ecb_size = 2, {{.ecc_blocks = 4, .data_words = 13}, {.ecc_blocks = 1, .data_words = 14}}},
    }},
[7] = {.alignment_pattern_num = 3, .alignment_pattern_centers = {6, 24, 42}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 24, .qr_ecb_size = 1, {{.ecc_blocks = 2, .data_words = 97}, {.ecc_blocks = 0, .data_words = 0}}},
    [1] = {.ec_codewords_per_block = 22, .qr_ecb_size = 2, {{.ecc_blocks = 2, .data_words = 38}, {.ecc_blocks = 2, .data_words = 39}}},
    [2] = {.ec_codewords_per_block = 22, .qr_ecb_size = 2, {{.ecc_blocks = 4, .data_words = 18}, {.ecc_blocks = 2, .data_words = 19}}},
    [3] = {.ec_codewords_per_block = 26, .qr_ecb_size = 2, {{.ecc_blocks = 4, .data_words = 14}, {.ecc_blocks = 2, .data_words = 15}}},
    }},
[8] = {.alignment_pattern_num = 3, .alignment_pattern_centers = {6, 26, 46}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 30, .qr_ecb_size = 1, {{.ecc_blocks = 2, .data_words = 116}, {.ecc_blocks = 0, .data_words = 0}}},
    [1] = {.ec_codewords_per_block = 22, .qr_ecb_size = 2, {{.ecc_blocks = 3, .data_words = 36}, {.ecc_blocks = 2, .data_words = 37}}},
    [2] = {.ec_codewords_per_block = 20, .qr_ecb_size = 2, {{.ecc_blocks = 4, .data_words = 16}, {.ecc_blocks = 4, .data_words = 17}}},
    [3] = {.ec_codewords_per_block = 24, .qr_ecb_size = 2, {{.ecc_blocks = 4, .data_words = 12}, {.ecc_blocks = 4, .data_words = 13}}},
    }},
[9] = {.alignment_pattern_num = 3, .alignment_pattern_centers = {6, 28, 50}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 18, .qr_ecb_size = 2, {{.ecc_blocks = 2, .data_words = 68}, {.ecc_blocks = 2, .data_words = 69}}},
    [1] = {.ec_codewords_per_block = 26, .qr_ecb_size = 2, {{.ecc_blocks = 4, .data_words = 43}, {.ecc_blocks = 1, .data_words = 44}}},
    [2] = {.ec_codewords_per_block = 24, .qr_ecb_size = 2, {{.ecc_blocks = 6, .data_words = 19}, {.ecc_blocks = 2, .data_words = 20}}},
    [3] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 6, .data_words = 15}, {.ecc_blocks = 2, .data_words = 16}}},
    }},
[10] = {.alignment_pattern_num = 3, .alignment_pattern_centers = {6, 30, 54}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 20, .qr_ecb_size = 1, {{.ecc_blocks = 4, .data_words = 81}, {.ecc_blocks = 0, .data_words = 0}}},
    [1] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 1, .data_words = 50}, {.ecc_blocks = 4, .data_words = 51}}},
    [2] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 4, .data_words = 22}, {.ecc_blocks = 4, .data_words = 23}}},
    [3] = {.ec_codewords_per_block = 24, .qr_ecb_size = 2, {{.ecc_blocks = 3, .data_words = 12}, {.ecc_blocks = 8, .data_words = 13}}},
    }},
[11] = {.alignment_pattern_num = 3, .alignment_pattern_centers = {6, 32, 58}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 24, .qr_ecb_size = 2, {{.ecc_blocks = 2, .data_words = 92}, {.ecc_blocks = 2, .data_words = 93}}},
    [1] = {.ec_codewords_per_block = 22, .qr_ecb_size = 2, {{.ecc_blocks = 6, .data_words = 36}, {.ecc_blocks = 2, .data_words = 37}}},
    [2] = {.ec_codewords_per_block = 26, .qr_ecb_size = 2, {{.ecc_blocks = 4, .data_words = 20}, {.ecc_blocks = 6, .data_words = 21}}},
    [3] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 7, .data_words = 14}, {.ecc_blocks = 4, .data_words = 15}}},
    }},
[12] = {.alignment_pattern_num = 3, .alignment_pattern_centers = {6, 34, 62}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 26, .qr_ecb_size = 1, {{.ecc_blocks = 4, .data_words = 107}, {.ecc_blocks = 0, .data_words = 0}}},
    [1] = {.ec_codewords_per_block = 22, .qr_ecb_size = 2, {{.ecc_blocks = 8, .data_words = 37}, {.ecc_blocks = 1, .data_words = 38}}},
    [2] = {.ec_codewords_per_block = 24, .qr_ecb_size = 2, {{.ecc_blocks = 8, .data_words = 20}, {.ecc_blocks = 4, .data_words = 21}}},
    [3] = {.ec_codewords_per_block = 22, .qr_ecb_size = 2, {{.ecc_blocks = 12, .data_words = 11}, {.ecc_blocks = 4, .data_words = 12}}},
    }},
[13] = {.alignment_pattern_num = 4, .alignment_pattern_centers = {6, 26, 46, 66}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 3, .data_words = 115}, {.ecc_blocks = 1, .data_words = 116}}},
    [1] = {.ec_codewords_per_block = 24, .qr_ecb_size = 2, {{.ecc_blocks = 4, .data_words = 40}, {.ecc_blocks = 5, .data_words = 41}}},
    [2] = {.ec_codewords_per_block = 20, .qr_ecb_size = 2, {{.ecc_blocks = 11, .data_words = 16}, {.ecc_blocks = 5, .data_words = 17}}},
    [3] = {.ec_codewords_per_block = 24, .qr_ecb_size = 2, {{.ecc_blocks = 11, .data_words = 12}, {.ecc_blocks = 5, .data_words = 13}}},
    }},
[14] = {.alignment_pattern_num = 4, .alignment_pattern_centers = {6, 26, 48, 70}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 22, .qr_ecb_size = 2, {{.ecc_blocks = 5, .data_words = 87}, {.ecc_blocks = 1, .data_words = 88}}},
    [1] = {.ec_codewords_per_block = 24, .qr_ecb_size = 2, {{.ecc_blocks = 5, .data_words = 41}, {.ecc_blocks = 5, .data_words = 42}}},
    [2] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 5, .data_words = 24}, {.ecc_blocks = 7, .data_words = 25}}},
    [3] = {.ec_codewords_per_block = 24, .qr_ecb_size = 2, {{.ecc_blocks = 11, .data_words = 12}, {.ecc_blocks = 7, .data_words = 13}}},
    }},
[15] = {.alignment_pattern_num = 4, .alignment_pattern_centers = {6, 26, 50, 74}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 24, .qr_ecb_size = 2, {{.ecc_blocks = 5, .data_words = 98}, {.ecc_blocks = 1, .data_words = 99}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 7, .data_words = 45}, {.ecc_blocks = 3, .data_words = 46}}},
    [2] = {.ec_codewords_per_block = 24, .qr_ecb_size = 2, {{.ecc_blocks = 15, .data_words = 19}, {.ecc_blocks = 2, .data_words = 20}}},
    [3] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 3, .data_words = 15}, {.ecc_blocks = 13, .data_words = 16}}},
    }},
[16] = {.alignment_pattern_num = 4, .alignment_pattern_centers = {6, 30, 54, 78}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 1, .data_words = 107}, {.ecc_blocks = 5, .data_words = 108}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 10, .data_words = 46}, {.ecc_blocks = 1, .data_words = 47}}},
    [2] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 1, .data_words = 22}, {.ecc_blocks = 15, .data_words = 23}}},
    [3] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 2, .data_words = 14}, {.ecc_blocks = 17, .data_words = 15}}},
    }},
[17] = {.alignment_pattern_num = 4, .alignment_pattern_centers = {6, 30, 56, 82}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 5, .data_words = 120}, {.ecc_blocks = 1, .data_words = 121}}},
    [1] = {.ec_codewords_per_block = 26, .qr_ecb_size = 2, {{.ecc_blocks = 9, .data_words = 43}, {.ecc_blocks = 4, .data_words = 44}}},
    [2] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 17, .data_words = 22}, {.ecc_blocks = 1, .data_words = 23}}},
    [3] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 2, .data_words = 14}, {.ecc_blocks = 19, .data_words = 15}}},
    }},
[18] = {.alignment_pattern_num = 4, .alignment_pattern_centers = {6, 30, 58, 86}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 3, .data_words = 113}, {.ecc_blocks = 4, .data_words = 114}}},
    [1] = {.ec_codewords_per_block = 26, .qr_ecb_size = 2, {{.ecc_blocks = 3, .data_words = 44}, {.ecc_blocks = 11, .data_words = 45}}},
    [2] = {.ec_codewords_per_block = 26, .qr_ecb_size = 2, {{.ecc_blocks = 17, .data_words = 21}, {.ecc_blocks = 4, .data_words = 22}}},
    [3] = {.ec_codewords_per_block = 26, .qr_ecb_size = 2, {{.ecc_blocks = 9, .data_words = 13}, {.ecc_blocks = 16, .data_words = 14}}},
    }},
[19] = {.alignment_pattern_num = 4, .alignment_pattern_centers = {6, 34, 62, 90}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 3, .data_words = 107}, {.ecc_blocks = 5, .data_words = 108}}},
    [1] = {.ec_codewords_per_block = 26, .qr_ecb_size = 2, {{.ecc_blocks = 3, .data_words = 41}, {.ecc_blocks = 13, .data_words = 42}}},
    [2] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 15, .data_words = 24}, {.ecc_blocks = 5, .data_words = 25}}},
    [3] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 15, .data_words = 15}, {.ecc_blocks = 10, .data_words = 16}}},
    }},
[20] = {.alignment_pattern_num = 5, .alignment_pattern_centers = {6, 28, 50, 72, 94}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 4, .data_words = 116}, {.ecc_blocks = 4, .data_words = 117}}},
    [1] = {.ec_codewords_per_block = 26, .qr_ecb_size = 1, {{.ecc_blocks = 17, .data_words = 42}, {.ecc_blocks = 0, .data_words = 0}}},
    [2] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 17, .data_words = 22}, {.ecc_blocks = 6, .data_words = 23}}},
    [3] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 19, .data_words = 16}, {.ecc_blocks = 6, .data_words = 17}}},
    }},
[21] = {.alignment_pattern_num = 5, .alignment_pattern_centers = {6, 26, 50, 74, 98}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 2, .data_words = 111}, {.ecc_blocks = 7, .data_words = 112}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 1, {{.ecc_blocks = 17, .data_words = 46}, {.ecc_blocks = 0, .data_words = 0}}},
    [2] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 7, .data_words = 24}, {.ecc_blocks = 16, .data_words = 25}}},
    [3] = {.ec_codewords_per_block = 24, .qr_ecb_size = 1, {{.ecc_blocks = 34, .data_words = 13}, {.ecc_blocks = 0, .data_words = 0}}},
    }},
[22] = {.alignment_pattern_num = 5, .alignment_pattern_centers = {6, 30, 54, 78, 102}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 4, .data_words = 121}, {.ecc_blocks = 5, .data_words = 122}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 4, .data_words = 47}, {.ecc_blocks = 14, .data_words = 48}}},
    [2] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 11, .data_words = 24}, {.ecc_blocks = 14, .data_words = 25}}},
    [3] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 16, .data_words = 15}, {.ecc_blocks = 14, .data_words = 16}}},
    }},
[23] = {.alignment_pattern_num = 5, .alignment_pattern_centers = {6, 28, 54, 80, 106}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 6, .data_words = 117}, {.ecc_blocks = 4, .data_words = 118}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 6, .data_words = 45}, {.ecc_blocks = 14, .data_words = 46}}},
    [2] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 11, .data_words = 24}, {.ecc_blocks = 16, .data_words = 25}}},
    [3] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 30, .data_words = 16}, {.ecc_blocks = 2, .data_words = 17}}},
    }},
[24] = {.alignment_pattern_num = 5, .alignment_pattern_centers = {6, 32, 58, 84, 110}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 26, .qr_ecb_size = 2, {{.ecc_blocks = 8, .data_words = 106}, {.ecc_blocks = 4, .data_words = 107}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 8, .data_words = 47}, {.ecc_blocks = 13, .data_words = 48}}},
    [2] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 7, .data_words = 24}, {.ecc_blocks = 22, .data_words = 25}}},
    [3] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 22, .data_words = 15}, {.ecc_blocks = 13, .data_words = 16}}},
    }},
[25] = {.alignment_pattern_num = 5, .alignment_pattern_centers = {6, 30, 58, 86, 114}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 10, .data_words = 114}, {.ecc_blocks = 2, .data_words = 115}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 19, .data_words = 46}, {.ecc_blocks = 4, .data_words = 47}}},
    [2] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 28, .data_words = 22}, {.ecc_blocks = 6, .data_words = 23}}},
    [3] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 33, .data_words = 16}, {.ecc_blocks = 4, .data_words = 17}}},
    }},
[26] = {.alignment_pattern_num = 5, .alignment_pattern_centers = {6, 34, 62, 90, 118}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 8, .data_words = 122}, {.ecc_blocks = 4, .data_words = 123}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 22, .data_words = 45}, {.ecc_blocks = 3, .data_words = 46}}},
    [2] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 8, .data_words = 23}, {.ecc_blocks = 26, .data_words = 24}}},
    [3] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 12, .data_words = 15}, {.ecc_blocks = 28, .data_words = 16}}},
    }},
[27] = {.alignment_pattern_num = 6, .alignment_pattern_centers = {6, 26, 50, 74, 98, 122}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 3, .data_words = 117}, {.ecc_blocks = 10, .data_words = 118}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 3, .data_words = 45}, {.ecc_blocks = 23, .data_words = 46}}},
    [2] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 4, .data_words = 24}, {.ecc_blocks = 31, .data_words = 25}}},
    [3] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 11, .data_words = 15}, {.ecc_blocks = 31, .data_words = 16}}},
    }},
[28] = {.alignment_pattern_num = 6, .alignment_pattern_centers = {6, 30, 54, 78, 102, 126}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 7, .data_words = 116}, {.ecc_blocks = 7, .data_words = 117}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 21, .data_words = 45}, {.ecc_blocks = 7, .data_words = 46}}},
    [2] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 1, .data_words = 23}, {.ecc_blocks = 37, .data_words = 24}}},
    [3] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 19, .data_words = 15}, {.ecc_blocks = 26, .data_words = 16}}},
    }},
[29] = {.alignment_pattern_num = 6, .alignment_pattern_centers = {6, 26, 52, 78, 104, 130}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 5, .data_words = 115}, {.ecc_blocks = 10, .data_words = 116}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 19, .data_words = 47}, {.ecc_blocks = 10, .data_words = 48}}},
    [2] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 15, .data_words = 24}, {.ecc_blocks = 25, .data_words = 25}}},
    [3] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 23, .data_words = 15}, {.ecc_blocks = 25, .data_words = 16}}},
    }},
[30] = {.alignment_pattern_num = 6, .alignment_pattern_centers = {6, 30, 56, 82, 108, 134}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 13, .data_words = 115}, {.ecc_blocks = 3, .data_words = 116}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 2, .data_words = 46}, {.ecc_blocks = 29, .data_words = 47}}},
    [2] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 42, .data_words = 24}, {.ecc_blocks = 1, .data_words = 25}}},
    [3] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 23, .data_words = 15}, {.ecc_blocks = 28, .data_words = 16}}},
    }},
[31] = {.alignment_pattern_num = 6, .alignment_pattern_centers = {6, 34, 60, 86, 112, 138}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 30, .qr_ecb_size = 1, {{.ecc_blocks = 17, .data_words = 115}, {.ecc_blocks = 0, .data_words = 0}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 10, .data_words = 46}, {.ecc_blocks = 23, .data_words = 47}}},
    [2] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 10, .data_words = 24}, {.ecc_blocks = 35, .data_words = 25}}},
    [3] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 19, .data_words = 15}, {.ecc_blocks = 35, .data_words = 16}}},
    }},
[32] = {.alignment_pattern_num = 6, .alignment_pattern_centers = {6, 30, 58, 86, 114, 142}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 17, .data_words = 115}, {.ecc_blocks = 1, .data_words = 116}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 14, .data_words = 46}, {.ecc_blocks = 21, .data_words = 47}}},
    [2] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 29, .data_words = 24}, {.ecc_blocks = 19, .data_words = 25}}},
    [3] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 11, .data_words = 15}, {.ecc_blocks = 46, .data_words = 16}}},
    }},
[33] = {.alignment_pattern_num = 6, .alignment_pattern_centers = {6, 34, 62, 90, 118, 146}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 13, .data_words = 115}, {.ecc_blocks = 6, .data_words = 116}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 14, .data_words = 46}, {.ecc_blocks = 23, .data_words = 47}}},
    [2] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 44, .data_words = 24}, {.ecc_blocks = 7, .data_words = 25}}},
    [3] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 59, .data_words = 16}, {.ecc_blocks = 1, .data_words = 17}}},
    }},
[34] = {.alignment_pattern_num = 7, .alignment_pattern_centers = {6, 30, 54, 78, 102, 126, 150}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 12, .data_words = 121}, {.ecc_blocks = 7, .data_words = 122}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 12, .data_words = 47}, {.ecc_blocks = 26, .data_words = 48}}},
    [2] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 39, .data_words = 24}, {.ecc_blocks = 14, .data_words = 25}}},
    [3] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 22, .data_words = 15}, {.ecc_blocks = 41, .data_words = 16}}},
    }},
[35] = {.alignment_pattern_num = 7, .alignment_pattern_centers = {6, 24, 50, 76, 102, 128, 154}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 6, .data_words = 121}, {.ecc_blocks = 14, .data_words = 122}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 6, .data_words = 47}, {.ecc_blocks = 34, .data_words = 48}}},
    [2] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 46, .data_words = 24}, {.ecc_blocks = 10, .data_words = 25}}},
    [3] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 2, .data_words = 15}, {.ecc_blocks = 64, .data_words = 16}}},
    }},
[36] = {.alignment_pattern_num = 7, .alignment_pattern_centers = {6, 28, 54, 80, 106, 132, 158}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 17, .data_words = 122}, {.ecc_blocks = 4, .data_words = 123}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 29, .data_words = 46}, {.ecc_blocks = 14, .data_words = 47}}},
    [2] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 49, .data_words = 24}, {.ecc_blocks = 10, .data_words = 25}}},
    [3] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 24, .data_words = 15}, {.ecc_blocks = 46, .data_words = 16}}},
    }},
[37] = {.alignment_pattern_num = 7, .alignment_pattern_centers = {6, 32, 58, 84, 110, 136, 162}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 4, .data_words = 122}, {.ecc_blocks = 18, .data_words = 123}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 13, .data_words = 46}, {.ecc_blocks = 32, .data_words = 47}}},
    [2] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 48, .data_words = 24}, {.ecc_blocks = 14, .data_words = 25}}},
    [3] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 42, .data_words = 15}, {.ecc_blocks = 32, .data_words = 16}}},
    }},
[38] = {.alignment_pattern_num = 7, .alignment_pattern_centers = {6, 26, 54, 82, 110, 138, 166}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 20, .data_words = 117}, {.ecc_blocks = 4, .data_words = 118}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 40, .data_words = 47}, {.ecc_blocks = 7, .data_words = 48}}},
    [2] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 43, .data_words = 24}, {.ecc_blocks = 22, .data_words = 25}}},
    [3] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 10, .data_words = 15}, {.ecc_blocks = 67, .data_words = 16}}},
    }},
[39] = {.alignment_pattern_num = 7, .alignment_pattern_centers = {6, 30, 58, 86, 114, 142, 170}, .ecblocks = {
    [0] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 19, .data_words = 118}, {.ecc_blocks = 6, .data_words = 119}}},
    [1] = {.ec_codewords_per_block = 28, .qr_ecb_size = 2, {{.ecc_blocks = 18, .data_words = 47}, {.ecc_blocks = 31, .data_words = 48}}},
    [2] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 34, .data_words = 24}, {.ecc_blocks = 34, .data_words = 25}}},
    [3] = {.ec_codewords_per_block = 30, .qr_ecb_size = 2, {{.ecc_blocks = 20, .data_words = 15}, {.ecc_blocks = 61, .data_words = 16}}},
    }}};

unsigned int qr_code_ncodewords(const unsigned int _version)
{
    unsigned int nalign;

    if (_version == 1)
        return 26;

    nalign = (_version / 7) + 2;

    return ((_version << 4) * (_version + 8) - (5 * nalign) * (5 * nalign - 2) + 36 * (_version < 7) + 83) >> 3;
}

/**
 * @brief qr_alignment_not_detect 防止校正图形和探测图形以及版本信息去重合
 * @param r 横坐标
 * @param c 列坐标
 * @param dimension 矩阵的维度
 */
static inline unsigned char qr_alignment_not_detect(const int r, const int c, const int dimension)
{
    return !((r <= 6 && (c <= 6 || c >= dimension - 10)) || (r >= dimension - 13 && c <= 10));
}

struct bitmatrix *qr_build_function_pattern(const unsigned int version)
{
    const unsigned int dimension = (version << 2) + 17;
    const unsigned int version_index = version - 1;
    struct bitmatrix *qr_function_matrix;
    unsigned int x, y;

    if (version == 0 || version > 40)
        return NULL;

    qr_function_matrix = bitmatrix_create(dimension, dimension, 0);
    if (qr_function_matrix == NULL)
        return NULL;

    /* 顶部两块探测图形区域、格式信息区域和空白区设置为1 */
    for (y = 0; y < 9; ++y) {
        for (x = 0; x < 9; ++x)
            bitmatrix_set(qr_function_matrix, x, y, 1);
        for (x = dimension - 8; x < dimension; ++x)
            bitmatrix_set(qr_function_matrix, x, y, 1);
    }

    /* 左下角探测图形区域、格式信息区域和空白区设置为1 */
    for (y = dimension - 8; y < dimension; ++y) {
        for (x = 0; x < 9; ++x)
            bitmatrix_set(qr_function_matrix, x, y, 1);
    }

    /* 定位图形全部设置为1 */
    for (y = 6, x = 9; x < dimension - 8; ++x) {
        bitmatrix_set(qr_function_matrix, x, y, 1);
        bitmatrix_set(qr_function_matrix, y, x, 1);
    }

    if (version > 1) {
        for (unsigned int j, i = 0; i < (unsigned int)qr_version_blocks[version_index].alignment_pattern_num; ++i) {
            for (j = 0; j < (unsigned int)qr_version_blocks[version_index].alignment_pattern_num; ++j) {
                if (!qr_alignment_not_detect(qr_version_blocks[version_index].alignment_pattern_centers[i],
                    qr_version_blocks[version_index].alignment_pattern_centers[j], dimension)) {
                    continue;
                }

                for (x = (unsigned int)qr_version_blocks[version_index].alignment_pattern_centers[i] - 2;
                        x <= (unsigned int)qr_version_blocks[version_index].alignment_pattern_centers[i] + 2;
                        ++x) {
                    for (y = (unsigned int)qr_version_blocks[version_index].alignment_pattern_centers[j] - 2;
                            y <= (unsigned int)qr_version_blocks[version_index].alignment_pattern_centers[j] + 2; ++y) {
                        bitmatrix_set(qr_function_matrix, x, y, 1);
                    }
                }
            }
        }
    }

    if (version > 6) {
        for (y = dimension - 11; y <= dimension - 8; ++y) {
            for (x = 0; x < 6; ++x) {
                bitmatrix_set(qr_function_matrix, y, x, 1);
                bitmatrix_set(qr_function_matrix, x, y, 1);
            }
        }
    }

    return qr_function_matrix;
}

struct circular_queue *qr_data_block_split(const unsigned int version, const unsigned int error_correction_level,
    const unsigned char *rawcodes)
{
    struct circular_queue *q;
    struct qr_data_block **data_blocks;
    unsigned int total_blocks;
    unsigned int ecc_code_offset;       /* 纠错码数据段中的偏移 */
    unsigned char max_data_codes;        /* 记录最长的数据码字的长度 */
    unsigned char ecc_codes;
    unsigned char ecc_level;
    unsigned int offset, j, i;
    const struct qr_ecblocks *ecblk;

    if (version < 1 || version > 40 || rawcodes == NULL)
        return NULL;

    /* 先计算出数据的块数 */
    total_blocks = 0;
    ecc_level = error_correction_level ^ 1;
    ecblk = qr_version_blocks[version - 1].ecblocks + ecc_level;
    for (int i = 0; i < ecblk->qr_ecb_size; ++i)
        total_blocks += ecblk->ecb[i].ecc_blocks;

    data_blocks = (struct qr_data_block **)mem_alloc(sizeof(struct qr_data_block *) * total_blocks);
    if (data_blocks == NULL)
        return NULL;

    q = circular_queue_create(total_blocks, sizeof(struct qr_data_block *));
    if (q == NULL) {
        mem_free(data_blocks);
        return NULL;
    }

    /* 为每一块码字分配内存 */
    ecc_code_offset = 0;
    max_data_codes = 0;
    ecc_codes = ecblk->ec_codewords_per_block;
    for (offset = 0, i = 0; i < ecblk->qr_ecb_size; ++i) {
        ecc_code_offset += ecblk->ecb[i].data_words * ecblk->ecb[i].ecc_blocks;
        if (ecblk->ecb[i].data_words > max_data_codes)
            max_data_codes = ecblk->ecb[i].data_words;

        for (j = 0; j < ecblk->ecb[i].ecc_blocks; ++j) {
            /* 先计算出码字的长度 */
            unsigned int codes = ecblk->ecb[i].data_words + ecc_codes;

            data_blocks[offset] = (struct qr_data_block *)mem_alloc(sizeof(struct qr_data_block) + codes);
            if (data_blocks[offset] == NULL) {
                for (unsigned int r = 0; r < offset; ++r)
                    mem_free(data_blocks[r]);
                mem_free(data_blocks);
                circular_queue_free(q);
                return NULL;
            }
            data_blocks[offset]->data_codewords = ecblk->ecb[i].data_words;
            data_blocks[offset]->length = codes;
            ++offset;
        }
    }
    
    /* 先读出数据码字 */
    offset = 0;
    for (j = 0; j < max_data_codes; ++j) {
        for (i = 0; i < total_blocks; ++i) {
            if (j >= data_blocks[i]->data_codewords)
                continue;
            data_blocks[i]->codewords[j] = rawcodes[offset++];
        }
    }

    /* 接着再读取出纠错码字 */
    for (j = 0; j < ecc_codes; ++j) {
        for (i = 0; i < total_blocks; ++i) {
            unsigned ioffset = j + data_blocks[i]->data_codewords;

            if (ioffset >= data_blocks[i]->length)
                continue;
            data_blocks[i]->codewords[ioffset] = rawcodes[offset++];
        }
    }
    circular_queue_clear(q);
    circular_queue_enque(q, data_blocks, total_blocks);
    mem_free(data_blocks);

    return q;
}
