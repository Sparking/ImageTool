#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "port_memory.h"

void linkedlist_queue_init(struct linkedlist_queue *q, const unsigned int block_size)
{
    if (q != NULL) {
        linkedlist_init(&q->q, block_size);
    }
}

unsigned int linkedlist_queue_size(const struct linkedlist_queue *q)
{
    return q == NULL ? 0 : linkedlist_size(&q->q);
}

unsigned int linkedlist_queue_enque(struct linkedlist_queue *q, const void *data, const unsigned int size)
{
    if (q == NULL || data == NULL || size == 0)
        return 0;

    return linkedlist_add(&q->q, data, size);
}

unsigned int linkedlist_queue_deque(struct linkedlist_queue *q, void *data, const unsigned int size)
{
    unsigned int i;

    if (q == NULL || size == 0)
        return 0;

    for (i = 0; i < size; ++i) {
        if (linkedlist_delete_tail(&q->q, data) != 1)
            break;

        if (data != NULL) {
            data = (char *)data + q->q.block_size;
        }
    }

    return i;
}

unsigned int linkedlist_queue_front(const struct linkedlist_queue *q, void *data)
{
    void *ptr;

    if (q == NULL || (ptr = linkedlist_tail(&q->q)) == NULL)
        return 0;

    memcpy(data, ptr, q->q.block_size);

    return 1;
}

unsigned int linkedlist_queue_rear(const struct linkedlist_queue *q, void *data)
{
    void *ptr;

    if (q == NULL || (ptr = linkedlist_head(&q->q)) == NULL)
        return 0;

    memcpy(data, ptr, q->q.block_size);

    return 1;
}

void linkedlist_queue_clear(struct linkedlist_queue *q)
{
    if (q != NULL) {
        linkedlist_clear(&q->q);
    }
}

void *linkedlist_queue_iterator_init(struct linkedlist_queue *q, struct linkedlist_queue_iterator *it)
{
    if (q == NULL || it == NULL)
        return NULL;

    linkedlist_iterator_init(&q->q, &it->it);
    return linkedlist_iterator_move(&it->it, 1);
}

void *linkedlist_queue_iterator_move(struct linkedlist_queue_iterator *it, const unsigned int offset)
{
    return linkedlist_iterator_move(&it->it, offset);
}

void *linkedlist_queue_iterator_data(const struct linkedlist_queue_iterator *it)
{
    return linkedlist_iterator_data(&it->it);
}

struct circular_queue *circular_queue_create(const unsigned int size, const unsigned int block_size)
{
    const unsigned int queue_size = sizeof(struct circular_queue) + size * block_size;
    struct circular_queue *q;

    if (queue_size == sizeof(struct circular_queue))
        return NULL;

    q = (struct circular_queue *)mem_alloc(queue_size);
    if (q == NULL)
        return NULL;

    q->size = size;
    q->block_size = block_size;
    q->used_size = 0;
    q->front = 0;
    q->rear = 0;

    return q;
}

int circular_queue_full(const struct circular_queue *q)
{
    if (q == NULL)
        return 0;

    return q->size == q->used_size;
}

int circular_queue_empty(const struct circular_queue *q)
{
    if (q == NULL)
        return 0;

    return q->used_size == 0;
}

unsigned int circular_queue_used_size(const struct circular_queue *q)
{
    if (q == NULL)
        return 0;

    return q->used_size;
}

unsigned int circular_queue_avaiable_size(const struct circular_queue *q)
{
    if (q == NULL)
        return 0;

    return q->size - q->used_size;
}

unsigned int circular_queue_enque(struct circular_queue *q, const void *data, const unsigned int size)
{
    unsigned int i;

    if (q == NULL || data == NULL)
        return 0;

    for (i = 0; i < size; ++i) {
        if (circular_queue_full(q))
            break;

        memcpy(q->data + q->rear * q->block_size, data, q->block_size);
        data = (char *)data + q->block_size;
        q->rear = (q->rear + 1) % q->size;
        ++q->used_size;
    }

    return i;
}

unsigned int circular_queue_deque(struct circular_queue *q, void *data, const unsigned int size)
{
    unsigned int i;

    if (q == NULL)
        return 0;

    for (i = 0; i < size; ++i) {
        if (circular_queue_empty(q))
            break;

        if (data != NULL) {
            memcpy(data, q->data + q->front * q->block_size, q->block_size);
            data = (char *)data + q->block_size;
        }

        q->front = (q->front + 1) % q->size;
        --q->used_size;
    }

    return i;
}

int circular_queue_front(const struct circular_queue *q, void *data, const unsigned int size)
{
    unsigned int i, j;

    if (q == NULL)
        return 0;

    if (circular_queue_empty(q))
        return 0;

    for (i = 0, j = q->front; i < size; ++i) {
        if (circular_queue_empty(q))
            break;

        memcpy(data, q->data +j * q->block_size, q->block_size);
        data = (char *)data + q->block_size;
        j = (j + 1) % q->size;
    }

    return 1;
}

int circular_queue_rear(const struct circular_queue *q, void *data)
{
    unsigned int pos;

    if (q == NULL)
        return 0;

    if (circular_queue_empty(q))
        return 0;

    pos = (q->rear == 0 ? q->size : q->rear) - 1;
    memcpy(data, q->data + pos * q->block_size, q->block_size);

    return 1;
}

void circular_queue_clear(struct circular_queue *q)
{
    if (q == NULL)
        return;

    q->used_size = 0;
    q->front = 0;
    q->rear = 0;
}

void circular_queue_free(struct circular_queue *q)
{
    mem_free(q);
}
