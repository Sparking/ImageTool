#include <string.h>
#include <stdlib.h>
#include "bitmatrix.h"
#include "port_memory.h"

static unsigned int bitmatrix_row_size(unsigned int dimension)
{
    unsigned int row_size = dimension >> 3;

    if ((dimension & 0x07) != 0)
        row_size++;

    return row_size;
}

struct bitmatrix *bitmatrix_create(const unsigned int row, const unsigned int column, const unsigned char reset_bit)
{
    const unsigned char reset_value = reset_bit ? 0xFF : 0;
    const unsigned int row_size = bitmatrix_row_size(column);
    const unsigned int all_size = row_size * row;
    struct bitmatrix *matrix;

    matrix = (struct bitmatrix *)mem_alloc(sizeof(struct bitmatrix) + all_size);
    if (matrix == NULL)
        return NULL;

    matrix->row = row;
    matrix->column = column;
    matrix->row_size = row_size;
    matrix->size = all_size;
    memset(matrix->matrix, reset_value, all_size);

    return matrix;
}

struct bitmatrix *bitmatrix_dump(const struct bitmatrix *src)
{
    struct bitmatrix *new_matrix;

    if (src == NULL)
        return NULL;

    new_matrix = (struct bitmatrix *)mem_alloc(sizeof(struct bitmatrix) + src->size);
    if (new_matrix == NULL)
        return NULL;

    memcpy(new_matrix, src, sizeof(struct bitmatrix) + src->size);

    return new_matrix;
}

void bitmatrix_release(struct bitmatrix *matrix)
{
    mem_free(matrix);
}

unsigned char bitmatrix_set(struct bitmatrix *matrix, const unsigned int row, const unsigned int column,
        const unsigned char bit)
{
    unsigned char ret;
    unsigned char mask;
    unsigned int offset;

    if (matrix == NULL || row >= matrix->row || column >= matrix->column)
        return 0;

    offset = matrix->row_size * row + (column >> 3);
    mask = 1 << (column & 0x07);
    ret = matrix->matrix[offset] & mask;
    matrix->matrix[offset] &= ~mask;
    if (bit)
        matrix->matrix[offset] |= mask;

    return ret != 0;
}

unsigned char bitmatrix_flip(struct bitmatrix *matrix, const unsigned int row, const unsigned int column)
{
    unsigned char ret;
    unsigned char mask;
    unsigned int offset;

    if (matrix == NULL || row >= matrix->row || column >= matrix->column)
        return 0;

    offset = matrix->row_size * row + (column >> 3);
    mask = 1 << (column & 0x07);
    ret = matrix->matrix[offset] & mask;
    matrix->matrix[offset] &= ~mask;
    if (!ret)
        matrix->matrix[offset] |= mask;

    return ret != 0;
}

unsigned char bitmatrix_xor(struct bitmatrix *matrix, const unsigned int row, const unsigned int column,
        const unsigned char bit)
{
    unsigned data;
    unsigned char ret;
    unsigned char mask;
    unsigned int offset;

    if (matrix == NULL || row >= matrix->row || column >= matrix->column)
        return 0;

    offset = matrix->row_size * row + (column >> 3);
    mask = 1 << (column & 0x07);
    ret = matrix->matrix[offset] & mask;
    data = (bit ? mask : 0) ^ ret;
    matrix->matrix[offset] &= ~mask;
    matrix->matrix[offset] |= data;

    return ret != 0;
}

unsigned char bitmatrix_get(const struct bitmatrix *matrix, const unsigned int row, const unsigned int column)
{
    unsigned char mask;
    unsigned int offset;

    if (matrix == NULL || row >= matrix->row || column >= matrix->column)
        return 0;

    offset = matrix->row_size * row + (column >> 3);
    mask = 1 << (column & 0x07);
    
    return (matrix->matrix[offset] & mask) != 0;
}
