#pragma once
#include <stdint.h>

struct bitstream {
    unsigned int bits;      /* 总位数 */
    unsigned int bytes;     /* 总字节数 */

    unsigned char *data;   /* 位流数据 */
};

/* 与位流关联的位流读取器 */
struct bitstream_reader {
    const struct bitstream *s;
    unsigned int current_bits_offset;
    unsigned int current_byte_offset;
    unsigned char current_bit_mask;
};

/**
 * @brief bitstream_create 创建一个位流
 * @param bits_length 位流的长度
 */
extern struct bitstream *bitstream_create(const unsigned int bits_length);

/**
 * @brief bitstream_create_from_bytes 从字节数据中创建一些位流
 * @param bytes 字节数据
 * @param bits_length 字节数据对应的位流长度
 */
extern struct bitstream *bitstream_create_from_bytes(const unsigned char *bytes,
        const unsigned int bits_length);

/**
 * @brief bitstream_set 设置位流中的某一位的值
 * @param s
 * @param bit_offset 被设置位在位流中的位置
 * @param bit 指定值
 * @return 返回被设置前的值, 但如果被设置的位置超出界限, 将返回0xFF
 */
extern unsigned char bitstream_set(struct bitstream *s, const unsigned int bit_offset,
        const unsigned char bit);

/**
 * @brief bitstream_get 读取位流中的某一位的值
 * @param s 位流
 * @param bit_offset 位在位流中的位置
 * @return 返回位的值, 如果越界访问将返回0xFF
 */
extern unsigned char bitstream_get(const struct bitstream *s, const unsigned int bit_offset);

/**
 * @brief bitstream_append 向位流尾部添加新的位流数据
 * @param s 位流
 * @param bits 新的位流数据
 * @param bits_length 新位流的长度
 * @return 返回新添加的位流长度
 */
extern unsigned int bitstream_append(struct bitstream *s, const unsigned char *bits,
        const unsigned int bits_length);

/**
 * @brief bitstream_release 释放位流占用的内存
 * @param s 位流
 */
extern void bitstream_release(struct bitstream *s);

/**
 * @brief bitstream_reader_init 将位流读取器和位流关联起来
 */
extern void bitstream_reader_init(const struct bitstream *s, struct bitstream_reader *reader);

/**
 * @brief bitstream_read 将位流读取器和位流关联起来
 */
extern unsigned int bitstream_read(struct bitstream_reader *reader,
        void *data, const unsigned int bits);

/**
 * @brief 位流读取器还可以读取的位的格式
 */
extern unsigned int bitstream_reader_avaiable_bits(struct bitstream_reader *reader);

