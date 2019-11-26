#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "maths.h"
#include "queue.h"
#include "qr_decoder.h"
#include "qr_version.h"
#include "qr_mode.h"
#include "rsdecode.h"
#include "port_memory.h"

#define FORMAT_INFO_MASK_QR     0x5412

#pragma pack(push, 1)
static struct {
    uint16_t data;
    unsigned char index;
}   FORMAT_INFO_DECODE_LOOKUP[32] = {
    {0x5412, 0x00}, {0x5125, 0x01}, {0x5E7C, 0x02}, {0x5B4B, 0x03}, {0x45F9, 0x04}, {0x40CE, 0x05},
    {0x4F97, 0x06}, {0x4AA0, 0x07}, {0x77C4, 0x08}, {0x72F3, 0x09}, {0x7DAA, 0x0A}, {0x789D, 0x0B},
    {0x662F, 0x0C}, {0x6318, 0x0D}, {0x6C41, 0x0E}, {0x6976, 0x0F}, {0x1689, 0x10}, {0x13BE, 0x11},
    {0x1CE7, 0x12}, {0x19D0, 0x13}, {0x0762, 0x14}, {0x0255, 0x15}, {0x0D0C, 0x16}, {0x083B, 0x17},
    {0x355F, 0x18}, {0x3068, 0x19}, {0x3F31, 0x1A}, {0x3A06, 0x1B}, {0x24B4, 0x1C}, {0x2183, 0x1D},
    {0x2EDA, 0x1E}, {0x2BED, 0x1F}};
#pragma pack(pop)

static unsigned int VERSION_DECODE_INFO[34] = {
    0x07C94, 0x085BC, 0x09A99, 0x0A4D3, 0x0BBF6, 0x0C762, 0x0D847, 0x0E60D, 0x0F928, 0x10B78,
    0x1145D, 0x12A17, 0x13532, 0x149A6, 0x15683, 0x168C9, 0x177EC, 0x18EC4, 0x191E1, 0x1AFAB,
    0x1B08E, 0x1CC1A, 0x1D33F, 0x1ED75, 0x1F250, 0x209D5, 0x216F0, 0x228BA, 0x2379F, 0x24B0B,
    0x2542E, 0x26A64, 0x27541, 0x28C69
};

static struct generic_gf *qr_gf = NULL;

static unsigned char do_format_information_decode(const uint16_t maskedFormation1, const uint16_t maskedFormation2)
{
    unsigned int best_difference = 0xFFFF;
    unsigned char best_format_info = 0;

    for (int i = 0; i < 32; ++i) {
        uint16_t target_info = FORMAT_INFO_DECODE_LOOKUP[i].data;

        if (target_info == maskedFormation1 || target_info == maskedFormation2) {
            return FORMAT_INFO_DECODE_LOOKUP[i].index;
        }

        uint16_t bits_difference = bits_count(maskedFormation1 ^ target_info);
        if (bits_difference < best_difference) {
            best_format_info = FORMAT_INFO_DECODE_LOOKUP[i].index;
            best_difference = bits_difference;
        }

        if (maskedFormation1 != maskedFormation2) {
            bits_difference = bits_count(maskedFormation2 ^ target_info);
            if (bits_difference < best_difference) {
                best_format_info = FORMAT_INFO_DECODE_LOOKUP[i].index ;
                best_difference = bits_difference;
            }
        }
    }

    if (best_difference <= 3)
        return best_format_info;

    return 0xFF;
}

static unsigned char qr_format_information_decode(const unsigned int maskedFormation1, const unsigned int maskedFormation2)
{
    unsigned char res = do_format_information_decode(maskedFormation1, maskedFormation2);

    if (res == 0xFF) {
        res = do_format_information_decode(maskedFormation1 ^ FORMAT_INFO_MASK_QR, maskedFormation2 ^ FORMAT_INFO_MASK_QR);
    }

    return res;
}

static unsigned char qr_version_information_decode(unsigned int version_bits)
{
    unsigned int best_difference = 0xFFFFFFFF;
    unsigned char best_version = 0;

    for (int i = 0; i < 34; ++i) {
        int target_version = VERSION_DECODE_INFO[i];
        if (target_version == version_bits)
            return i + 7;

        unsigned int bits_difference = bits_count(version_bits ^ target_version);
        if (bits_difference < best_difference) {
            best_version = i + 7;
            best_difference = bits_difference;
        }
    }

    if (best_difference <= 3)
        return best_version;

    return 0xFF;
}

static unsigned char qr_mask_condtion0(const int i, const int j)
{
    return ((i + j) & 1) == 0;
}

static unsigned char qr_mask_condtion1(const int i, const int j)
{
    return (i & 1) == 0;
}

static unsigned char qr_mask_condtion2(const int i, const int j)
{
    return (j % 3) == 0;
}

static unsigned char qr_mask_condtion3(const int i, const int j)
{
    return ((i + j) % 3) == 0;
}

static unsigned char qr_mask_condtion4(const int i, const int j)
{
    return (((i >> 1) + (j / 3)) & 0x01) == 0;
}

static unsigned char qr_mask_condtion5(const int i, const int j)
{
    int r = i * j;

    return (r % 6) == 0;
}

static unsigned char qr_mask_condtion6(const int i, const int j)
{
    int r = i * j;

    return (r % 6) < 3;
}

static unsigned char qr_mask_condtion7(const int i, const int j)
{
    return ((((i * j) % 3) + i + j) & 1) == 0;
}

static unsigned char (*const qr_mask_condtion[8])(const int, const int) = {
    qr_mask_condtion0, qr_mask_condtion1, qr_mask_condtion2, qr_mask_condtion3,
    qr_mask_condtion4, qr_mask_condtion5, qr_mask_condtion6, qr_mask_condtion7};

int qr_decode_init(void)
{
    if (qr_gf != NULL)
        return 0;

    return ((qr_gf = generic_gf_create(0x011D, 256, 0)) != NULL) ? 0 : -1;    
}

void qr_decode_deinit(void)
{
    if (qr_gf != NULL) {
        generic_gf_release(qr_gf);
        qr_gf = NULL;
    }
}

struct qr_decode_info *qr_decode(struct bitmatrix *sample_image)
{
    unsigned char status;
    struct qr_decode_info *parse_result;

    if (qr_gf == NULL || sample_image == NULL)
        return NULL;

    parse_result = (struct qr_decode_info *)mem_alloc(sizeof(struct qr_decode_info));
    if (parse_result == NULL)
        return NULL;

    status = 0;
    parse_result->error_correct_level = 0;
    parse_result->mask = 0;
    parse_result->data = NULL;
    parse_result->data_size = 0;
    parse_result->version = (sample_image->row - 17) >> 2;
    if (parse_result->version > 40) {
        goto release_resource;
    }

    {
        uint16_t qr_code_format_mesg[2] = {0, 0};
        unsigned int i, j;

        for (i = 0, j = 8; i <= 5; ++i) {
            qr_code_format_mesg[0] >>= 1;
            qr_code_format_mesg[0] += ((uint16_t)bitmatrix_get(sample_image, i, j)) << 14;
        }
        for (i = 7; i <= 8; ++i) {
            qr_code_format_mesg[0] >>= 1;
            qr_code_format_mesg[0] += ((uint16_t)bitmatrix_get(sample_image, i, j)) << 14;
        }
        i = 8;
        j = 7;
        qr_code_format_mesg[0] >>= 1;
        qr_code_format_mesg[0] += ((uint16_t)bitmatrix_get(sample_image, i, j)) << 14;
        for (j = 6; j-- > 0;) {
            qr_code_format_mesg[0] >>= 1;
            qr_code_format_mesg[0] += ((uint16_t)bitmatrix_get(sample_image, i, j)) << 14;
        }

        for (j = sample_image->row - 1; j >= sample_image->row - 8; --j) {
            qr_code_format_mesg[1] >>= 1;
            qr_code_format_mesg[1] += ((uint16_t)bitmatrix_get(sample_image, i, j)) << 14;
        }

        for (i = sample_image->row - 7, j = 8; i < sample_image->row; ++i) {
            qr_code_format_mesg[1] >>= 1;
            qr_code_format_mesg[1] += ((uint16_t)bitmatrix_get(sample_image, i, j)) << 14;
        }

        unsigned char decode_result = qr_format_information_decode(qr_code_format_mesg[0], qr_code_format_mesg[1]);
        if (decode_result == 0xFF) {
            goto release_resource;
        } else {
            parse_result->error_correct_level = decode_result >> 3;
            parse_result->mask = decode_result & 0x7;
        }
    }

    if (parse_result->version >= 7) {
        unsigned int qr_code_version_mesg[2] = {0, 0};
        unsigned int i, j;

        for (j = 0; j < 6; ++j) {
            for (i = sample_image->row - 11; i <= sample_image->row - 9; ++i) {
                qr_code_version_mesg[0] >>= 1;
                qr_code_version_mesg[0] += ((unsigned int)bitmatrix_get(sample_image, i, j)) << 17;
            }
        }

        for (i = 0; i < 6; ++i) {
            for (j = sample_image->row - 11; j <= sample_image->row - 9; ++j) {
                qr_code_version_mesg[1] >>= 1;
                qr_code_version_mesg[1] += ((unsigned int)bitmatrix_get(sample_image, i, j)) << 17;
            }
        }

        unsigned char version = qr_version_information_decode(qr_code_version_mesg[0]);
        if (version > 40) {
            version = qr_version_information_decode(qr_code_version_mesg[1]);
        }
        if (version > 40) {
            goto release_resource;
        } else {
            if (version != parse_result->version) {
                goto release_resource;
            }
        }
    }
    {
        struct bitmatrix *function_pattern = qr_build_function_pattern(parse_result->version);
        if (function_pattern == NULL) {
            goto release_resource;
        }

        /* 消除掩膜 */
        for (unsigned int i = 0; i < sample_image->row; ++i) {
            for (unsigned int j = 0; j < sample_image->column; ++j) {
                bitmatrix_xor(sample_image, i, j, qr_mask_condtion[parse_result->mask](i, j));
            }
        }

        /* 读取码字信息 */
        {
            const unsigned int code_ncodewords = qr_code_ncodewords(parse_result->version);
            unsigned char *codewords = (unsigned char *)mem_alloc(code_ncodewords);
            if (codewords == NULL) {
                bitmatrix_release(function_pattern);
                goto release_resource;
            }

            unsigned int result_offset = 0;
            unsigned int bits_read = 0;
            unsigned char current_bytes = 0;
            unsigned char readingUp = 1;
            for (int j = sample_image->column - 1; j > 0; j -= 2) {
                if (j == 6)
                    --j;
                for (unsigned int count = 0; count < sample_image->row; ++count) {
                    unsigned int i = readingUp ? sample_image->row - 1 - count : count;
                    for (int col = 0; col < 2; ++col) {
                        if (!bitmatrix_get(function_pattern, i, j - col)) {
                            current_bytes <<= 1;
                            current_bytes |= bitmatrix_get(sample_image, i, j - col);
                            if (++bits_read == 8) {
                                if (result_offset >= code_ncodewords) {
                                    mem_free(codewords);
                                    bitmatrix_release(function_pattern);
                                    goto release_resource;
                                }
                                codewords[result_offset++] = (unsigned char)current_bytes;
                                bits_read = 0;
                                current_bytes = 0;
                            }
                        }
                    }
                }
                readingUp = !readingUp;
            }
            bitmatrix_release(function_pattern);
            if (result_offset != code_ncodewords) {
                mem_free(codewords);
                goto release_resource;
            }

            struct circular_queue *data_blokcs = qr_data_block_split(parse_result->version, parse_result->error_correct_level, codewords);
            if (data_blokcs == NULL) {
                mem_free(codewords);
                goto release_resource;
            }

            unsigned int offset = 0, offset1 = 0;
            while (!circular_queue_empty(data_blokcs)) {
                struct qr_data_block *blk;
                unsigned int *_data;

                circular_queue_deque(data_blokcs, &blk, 1);
                offset1 += blk->data_codewords;
                _data = (unsigned int *)mem_alloc(sizeof(unsigned int) * blk->length);
                if (_data == NULL) {
                    mem_free(blk);
                    continue;
                }

                for (unsigned int _off = 0; _off < blk->length; ++_off)
                    _data[_off] = blk->codewords[_off];

                if (rsdecode(qr_gf, _data, blk->length, blk->length - blk->data_codewords) == 0) {
                    for (unsigned int _off = 0; _off < blk->data_codewords;) {
                        codewords[offset++] = _data[_off++];
                    }
                }
                mem_free(_data);
                mem_free(blk);
            }
            circular_queue_free(data_blokcs);
            if (offset1 != offset) {
                mem_free(codewords);
                goto release_resource;
            }
            parse_result->data = codewords;
            parse_result->data_size = offset;
        }
    }

    {
        struct qr_bitstream_decode *qbsd = qr_bitstream_decode(parse_result->data, parse_result->data_size, parse_result->version);
        if (qbsd == NULL) {
            mem_free(parse_result->data);
            goto release_resource;
        }

        unsigned int offset = 0;
        while (qbsd != NULL) {
            void *temp;
            unsigned int i;

            for (i = 0; i < qbsd->size; ++i) {
                parse_result->data[offset] = qbsd->data[i];
                if (qbsd->mode == QR_MODE_NUMBERIC)
                    parse_result->data[offset] += '0';

                ++offset;
            }
            temp = qbsd;
            qbsd = qbsd->next;
            mem_free(temp);
        }
        parse_result->data[offset] = '\0';
        parse_result->data_size = offset;
    }

    status = 1;
release_resource:
    if (status == 0) {
        mem_free(parse_result);
        parse_result = NULL;
    }

    return parse_result;
}
