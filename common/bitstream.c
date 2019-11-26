#include <stdlib.h>
#include <string.h>
#include "bitstream.h"
#include "port_memory.h"

struct bitstream *bitstream_create(const unsigned int bits_length)
{
    struct bitstream *s;
    unsigned int bytes;

    s = (struct bitstream *)mem_alloc(sizeof(struct bitstream));
    if (s == NULL) {
        return NULL;
    }

    bytes = bits_length != 0 ? ((bits_length >> 7) + ((bits_length & 0x7F) != 0)) << 4 : 16;
    s->data = (unsigned char *)mem_alloc(sizeof(unsigned char) * bytes);
    if (s->data == NULL) {
        mem_free(s);
        return NULL;
    }

    s->bits = bits_length;
    s->bytes = bytes;
    memset(s->data, 0, bytes);

    return s;
}

unsigned int bitstream_append(struct bitstream *s, const unsigned char *bits, const unsigned int bits_length)
{
    unsigned int new_bytes, new_bits_length;
    unsigned char stream_bit_mask, stream_byte_offset;
    unsigned char src_bit_mask, src_byte_offset;

    if (s == NULL || bits == NULL)
        return 0;

    new_bits_length = s->bits + bits_length;
    new_bytes = ((new_bits_length >> 7) + ((new_bits_length & 0x7F) != 0)) << 4;
    if (new_bytes > s->bytes) {     /* 扩容 */
        unsigned char *new_data;

        new_data = (unsigned char *)mem_alloc(sizeof(unsigned char) * new_bytes);
        if (new_data == NULL)
            return 0;

        memcpy(new_data, s->data, s->bytes);
        mem_free(s->data);
        s->data = new_data;
        s->bytes = new_bytes;
    }

    src_byte_offset = 0;
    src_bit_mask = 0x80;
    stream_byte_offset = s->bits >> 3;
    stream_bit_mask = 1 << (s->bits & 0x07);
    while (s->bits < new_bits_length) {
        s->data[stream_byte_offset] &= ~stream_bit_mask;
        if (bits[src_byte_offset] & src_bit_mask) {
            s->data[stream_byte_offset] |= stream_bit_mask;
        }

        if (src_bit_mask == 0x01) {
            src_bit_mask = 0x80;
            ++src_byte_offset;
        } else {
            src_bit_mask >>= 1;
        }

        if (stream_bit_mask == 0x80) {
            stream_bit_mask = 1;
            ++stream_byte_offset;
        } else {
            stream_bit_mask <<= 1;
        }

        ++s->bits;
    }

    return bits_length;
}

struct bitstream *bitstream_create_from_bytes(const unsigned char *bytes, const unsigned int bits_length)
{
    struct bitstream *s;
    unsigned char src_bit_mask, dst_bit_mask;

    if (bytes == NULL)
        return NULL;

    src_bit_mask = 0x80;
    dst_bit_mask = 0x01;
    s = bitstream_create(bits_length);
    if (s != NULL) {
        for (unsigned int current_bits = 0, i = 0; current_bits < bits_length; ++current_bits) {
            if (bytes[i] & src_bit_mask)
                s->data[i] |= dst_bit_mask;

            if (src_bit_mask == 0x01) {
                dst_bit_mask = 0x01;
                src_bit_mask = 0x80;
                ++i;
            } else {
                dst_bit_mask <<= 1;
                src_bit_mask >>= 1;
            }
        }
    }

    return s;
}

void bitstream_release(struct bitstream *s)
{
    if (s != NULL) {
        if (s->data != NULL)
            mem_free(s->data);
        mem_free(s);
    }
}

unsigned char bitstream_set(struct bitstream *s, const unsigned int bit_offset, const unsigned char bit)
{
    unsigned char bit_mask, ret;
    unsigned int byte_offset;

    if (s == NULL || bit_offset >= s->bits)
        return 0xFF;

    byte_offset = bit_offset >> 3;
    bit_mask = 1 << (bit_offset & 0x07);
    ret = (s->data[byte_offset] & bit_mask) != 0;
    s->data[byte_offset] &= ~bit_mask;
    if (bit)
        s->data[byte_offset] |= ~bit_mask;

    return ret;
}

unsigned char bitstream_get(const struct bitstream *s, const unsigned int bit_offset)
{
    if (s == NULL || bit_offset >= s->bits)
        return 0xFF;

    return (s->data[bit_offset >> 3] >> (bit_offset & 0x07)) & 0x01;
}

void bitstream_reader_init(const struct bitstream *s, struct bitstream_reader *reader)
{
    if (reader == NULL)
        return;

    reader->current_bits_offset = 0;
    reader->current_byte_offset = 0;
    reader->current_bit_mask = 1;
    reader->s = s;
}

unsigned int bitstream_read(struct bitstream_reader *reader, void *pdata, const unsigned int bits)
{
    const struct bitstream *bs;
    unsigned char *data = pdata;
    unsigned int data_byte_offset;
    unsigned char data_bits_mask, bits_counter;

    if (reader == NULL || reader->s == NULL || bits == 0)
        return 0;

    bs = reader->s;
    data_byte_offset = (bits >> 3) + ((bits & 0x07) != 0);
    memset(data, 0, data_byte_offset);
    --data_byte_offset;
    data_bits_mask = 1 << ((bits - 1) & 0x07);
    for (bits_counter = 0; bits_counter < bits; ++bits_counter) {
        if (reader->current_bits_offset >= bs->bits)
            break;

        if (bs->data[reader->current_byte_offset] & reader->current_bit_mask)
            data[data_byte_offset] |= data_bits_mask;

        if (data_bits_mask == 0x01) {
            --data_byte_offset;
            data_bits_mask = 0x80;
        } else {
            data_bits_mask >>= 1;
        }

        if (reader->current_bit_mask == 0x80) {
            ++reader->current_byte_offset;
            reader->current_bit_mask = 1;
        } else {
            reader->current_bit_mask <<= 1;
        }
        ++reader->current_bits_offset;
    }

    return bits_counter;
}

unsigned int bitstream_reader_avaiable_bits(struct bitstream_reader *reader)
{
    if (reader == NULL || reader->s == NULL)
        return 0;

    return reader->s->bits - reader->current_bits_offset;
}
