#pragma once

#include "bitmatrix.h"
#include "queue.h"

struct qr_alignment_pattern {
    unsigned int row;
    unsigned int column;
    float estimated_module_size;
};

extern struct qr_alignment_pattern *qr_alignment_pattern_find(struct linkedlist_queue *pattern_queue,
        const struct bitmatrix *image_matrix, const unsigned int start_row, const unsigned int start_column,
        const unsigned int width, const unsigned int height, const float module_size);
