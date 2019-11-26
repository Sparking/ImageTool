#pragma once

struct bitmatrix {
    unsigned int row;       /* 行数 */
    unsigned int column;    /* 列数 */

    unsigned int row_size;  /* 每一行的字节长度 */
    unsigned int size;      /* 总大小 */
    char matrix[0];
};

/**
 * @brief bitmatrix_create 创建一个指定大小的二元矩阵
 * @param row 行数
 * @param column 列数
 * @param reset_bit 矩阵的全局复位值,0和1
 */
extern struct bitmatrix *bitmatrix_create(const unsigned int row,
		const unsigned int column, const unsigned char reset_bit);

/**
 * @brief bitmatrix_dump 复制二元矩阵
 */
extern struct bitmatrix *bitmatrix_dump(const struct bitmatrix *src);

/**
 * @brief bitmatrix_release 释放二元矩阵占用的内存
 */
extern void bitmatrix_release(struct bitmatrix *matrix);

/**
 * @brief bitmatrix_set 设置二元矩阵某一位的值
 * @param row 行数
 * @param column 列数
 * @param bit 矩阵元素的设定值, 0和1
 * @return 返回被设定之前的元素值
 */
extern unsigned char bitmatrix_set(struct bitmatrix *matrix, const unsigned int row,
		const unsigned int column, const unsigned char bit);

/**
 * @brief bitmatrix_xor 二元矩阵某一位进行异或
 * @param row 行数
 * @param column 列数
 * @param bit 和矩阵元素进行异或运算的值, 0和1
 * @return 返回异或运算之前的元素值
 */
extern unsigned char bitmatrix_xor(struct bitmatrix *matrix, const unsigned int row,
		const unsigned int column, const unsigned char bit);

/**
 * @brief bitmatrix_flip 二元矩阵某一位进行翻转
 * @param row 行数
 * @param column 列数
 * @return 返回翻转之前的元素值
 */
extern unsigned char bitmatrix_flip(struct bitmatrix *matrix, const unsigned int row,
		const unsigned int column);

/**
 * @brief bitmatrix_get 获取二元矩阵某一位的值
 * @param row 行数
 * @param column 列数
 */
extern unsigned char bitmatrix_get(const struct bitmatrix *matrix,
		const unsigned int row, const unsigned int column);
