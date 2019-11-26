#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "bitstream.h"
#include "qr_mode.h"
#include "port_memory.h"

static const unsigned char qr_numberic_counter_bits[3] = {10, 12, 14};
static const unsigned char qr_alnum_counter_bits[3]    = { 9, 11, 13};
static const unsigned char qr_byte_counter_bits[3]     = { 8, 16, 16};
static const unsigned char qr_hanzi_counter_bits[3]    = { 8, 10, 12};

static const unsigned char qr_alnum_table[45] = {
  '0','1','2','3','4','5','6','7','8','9',
  'A','B','C','D','E','F','G','H','I','J',
  'K','L','M','N','O','P','Q','R','S','T',
  'U','V','W','X','Y','Z',' ','$','%','*',
  '+','-','.','/',':'
};

static int qr_bitstream_to_numberic(struct qr_bitstream_decode *qbs, struct bitstream_reader *br, const unsigned int counts)
{
    unsigned int current_counts;
    unsigned int current_data;
    unsigned int stream_data_offset;

    stream_data_offset = 0;
    current_counts = counts;
    while (current_counts >= 3) {
        current_data = 0;
        if (bitstream_read(br, &current_data, 10) != 10) {
            return -1;
        }
        qbs->data[stream_data_offset++] = (current_data % 1000) / 100;
        qbs->data[stream_data_offset++] = (current_data % 100) / 10;
        qbs->data[stream_data_offset++] = current_data % 10;
        current_counts -= 3;
    }

    current_data = 0;
    switch (current_counts) {
    case 1:
        if (bitstream_read(br, &current_data, 4) != 4) {
            return -1;
        }
        qbs->data[stream_data_offset++] = current_data % 10;
        break;
    case 2:
        if (bitstream_read(br, &current_data, 7) != 7) {
            return -1;
        }
        qbs->data[stream_data_offset++] = (current_data % 100) / 10;
        qbs->data[stream_data_offset++] = current_data % 10;
        break;
    }

    return 0;
}

static int qr_bitstream_to_alnum(struct qr_bitstream_decode *qbs, struct bitstream_reader *br, const unsigned int counts)
{
    unsigned int current_counts;
    unsigned int current_data;
    unsigned int stream_data_offset;

    stream_data_offset = 0;
    current_counts = counts;
    while (current_counts >= 2) {
        current_data = 0;
        if (bitstream_read(br, &current_data, 11) != 11) {
            return -1;
        }

        qbs->data[stream_data_offset++] = qr_alnum_table[current_data / 45];
        qbs->data[stream_data_offset++] = qr_alnum_table[current_data % 45];
        current_counts -= 2;
    }

    if (current_counts > 0) {
        current_data = 0;
        if (bitstream_read(br, &current_data, 6) != 6) {
            return -1;
        }
        qbs->data[stream_data_offset++] = qr_alnum_table[current_data];
    }

    return 0;
}

static int qr_bitstream_to_byte(struct qr_bitstream_decode *qbs, struct bitstream_reader *br, const unsigned int counts)
{
    unsigned int current_counts;
    unsigned int stream_data_offset;
    unsigned char current_data;

    stream_data_offset = 0;
    current_counts = counts;
    while (current_counts-- > 0) {
        current_data = 0;
        if (bitstream_read(br, &current_data, 8) != 8) {
            return -1;
        }

        qbs->data[stream_data_offset++] = current_data;
    }

    return 0;
}

static int qr_bitstream_to_hanzi(struct qr_bitstream_decode *qbs, struct bitstream_reader *br, const unsigned int counts)
{
    unsigned int current_counts;
    unsigned int stream_data_offset;
    unsigned int current_data;
    unsigned int a, b;

    switch (qbs->submode) {
    case 0x0001:
        stream_data_offset = 0;
        current_counts = counts;
        while (current_counts-- > 0) {
            current_data = 0;
            if (bitstream_read(br, &current_data, 13) != 13) {
                return -1;
            }

            a = current_data / 0x60;
            b = current_data % 0x60;
            a += (a <= 0x09) ? 0xa1 : 0xa6;
            b += 0xa1;
            qbs->data[stream_data_offset++] = a;
            qbs->data[stream_data_offset++] = b;
        }
        break;
    default:
        return -1;
    }

    return 0;
}

struct qr_bitstream_decode *qr_bitstream_decode(const unsigned char *bytes,
    const unsigned int bytes_length, const unsigned char version)
{
    struct bitstream *bs;
    struct qr_bitstream_decode *s, *header, *prev;
    struct bitstream_reader bs_reader;
    unsigned int submode, char_counts, eci_task_id;
    unsigned char ccbi, mode, count_bits, count_unit, is_eci;
    int (*decode_func)(struct qr_bitstream_decode *, struct bitstream_reader *, const unsigned int);

    if (bytes == NULL || bytes_length == 0 || version > 40 || version < 1)
        return NULL;

    bs = bitstream_create_from_bytes(bytes, bytes_length << 3);
    if (bs == NULL)
        return NULL;

    header = prev = NULL;
    bitstream_reader_init(bs, &bs_reader);
    if (version < 10) {
        ccbi = 0;
    } else if (version < 27) {
        ccbi = 1;
    } else {
        ccbi = 2;
    }

    is_eci = 0;
    eci_task_id = 0;
    while (bitstream_reader_avaiable_bits(&bs_reader) > 0) {
        submode = 0;
        mode = 0;
        if (bitstream_read(&bs_reader, &mode, 4) != 4)
            goto release_resource;
        switch (mode) {
        case QR_MODE_NUMBERIC:
            count_bits = qr_numberic_counter_bits[ccbi];
            decode_func = qr_bitstream_to_numberic;
            count_unit = 1;
            break;
        case QR_MODE_ALPHANUMBERIC:
            count_bits = qr_alnum_counter_bits[ccbi];
            decode_func = qr_bitstream_to_alnum;
            count_unit = 1;
            break;
        case QR_MODE_BYTE:
            count_bits = qr_byte_counter_bits[ccbi];
            decode_func = qr_bitstream_to_byte;
            count_unit = 1;
            break;
        case QR_MODE_HANZI:
            count_bits = qr_hanzi_counter_bits[ccbi];
            decode_func = qr_bitstream_to_hanzi;
            if (bitstream_read(&bs_reader, &submode, 4) != 4)
                goto release_resource;
            count_unit = 2;
            break;
        case QR_MODE_ECI_MODE:
            if (bitstream_read(&bs_reader, &eci_task_id, 8) != 8)
                goto release_resource;
            if (eci_task_id & 0x40) {
                unsigned int temp = 0;
                if (bitstream_read(&bs_reader, &temp, 16) != 16)
                    goto release_resource;
                eci_task_id = ((eci_task_id & 0x1F) << 16) + temp;
            } else if (eci_task_id & 0x80) {
                unsigned int temp = 0;
                if (bitstream_read(&bs_reader, &temp, 8) != 8)
                    goto release_resource;
                eci_task_id = ((eci_task_id & 0x3F) << 8) + temp;
            }
            is_eci = 1;
            continue;
            break;
        case QR_MODE_TERMINATOR:
        case QR_MODE_FNC1_FIRST_POSITION:
        case QR_MODE_FNC1_SECOND_POSITION:
        default:
            goto release_resource;
            break;
        }

        char_counts = 0;
        if (bitstream_read(&bs_reader, &char_counts, count_bits) != count_bits) {
            goto release_resource;
        }

        s = (struct qr_bitstream_decode *)mem_alloc(sizeof(struct qr_bitstream_decode) + char_counts * count_unit);
        if (s == NULL) {
            s = header;
            while ((prev = s) != NULL) {
                s = s->next;
                mem_free(prev);
            }
            header = NULL;
            goto release_resource;
        }
        if (header == NULL) {
            header = s;
            prev = s;
        } else {
            prev->next = s;
            prev = s;
        }

        s->next = NULL;
        s->is_eci = is_eci;
        if (is_eci) {
            s->eci_task_id = eci_task_id;
            eci_task_id = 0;
            is_eci = 0;
        }
        s->mode = mode;
        s->submode = submode;
        s->size = char_counts * count_unit;
        memset(s->data, 0, s->size);
        decode_func(s, &bs_reader, char_counts);
    }

release_resource:
    bitstream_release(bs);
    return header;
}
