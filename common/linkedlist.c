#include <stdlib.h>
#include <string.h>
#include "linkedlist.h"
#include "port_memory.h"

struct linkedlist_node {
    struct list_head node;
    char data[0];
};

void linkedlist_init(struct linkedlist *list, const unsigned int block_size)
{
    if (list != NULL) {
        INIT_LIST_HEAD(list->head);
        list->block_size = block_size;
        list->size = 0;
    }
}

unsigned int linkedlist_size(const struct linkedlist *list)
{
    return list == NULL ? 0 : list->size;
}

static unsigned int linkedlist_inner_add(struct linkedlist *list, const void *data, const unsigned int size,
        const unsigned int flag)
{
    unsigned int i;
    struct linkedlist_node *node;

    if (list == NULL || data == NULL || size == 0)
        return 0;

    for (i = 0; i < size; ++i) {
        node = (struct linkedlist_node *)mem_alloc(sizeof(struct linkedlist_node) + list->block_size);
        if (node == NULL)
            break;

        INIT_LIST_HEAD(node->node);
        memcpy(node->data, data, list->block_size);
        data = (char *)data + list->block_size;
        if (flag) {
            list_add_tail(&node->node, &list->head);
        } else {
            list_add(&node->node, &list->head);
        }
        ++list->size;
    }

    return i;
}

unsigned int linkedlist_add(struct linkedlist *list, const void *data, const unsigned int size)
{
    return linkedlist_inner_add(list, data, size, 0);
}

unsigned int linkedlist_add_tail(struct linkedlist *list, const void *data, const unsigned int size)
{
    return linkedlist_inner_add(list, data, size, 1);
}

static unsigned int linkedlist_inner_delete(struct linkedlist *list, void *data, const unsigned int flag)
{
    struct linkedlist_node *node;
    struct list_head *list_node;

    if (list == NULL || list_empty(&list->head))
        return 0;

    list_node = (flag ? list->head.prev : list->head.next);
    node = list_entry(list_node, struct linkedlist_node, node);
    if (data != NULL) {
        memcpy(data, node->data, list->block_size);
        data = (char *)data + list->block_size;
    }
    list_del(&node->node);
    mem_free(node);
    --list->size;

    return 1;
}

unsigned int linkedlist_delete(struct linkedlist *list, void *data)
{
    return linkedlist_inner_delete(list, data, 0);
}

unsigned int linkedlist_delete_tail(struct linkedlist *list, void *data)
{
    return linkedlist_inner_delete(list, data, 1);
}

void *linkedlist_head(const struct linkedlist *list)
{
    if (list == NULL || list_empty(&list->head))
        return NULL;

    return list_entry(list->head.next, struct linkedlist_node, node)->data;
}

void *linkedlist_tail(const struct linkedlist *list)
{
    if (list == NULL || list_empty(&list->head))
        return NULL;

    return list_entry(list->head.prev, struct linkedlist_node, node)->data;
}

void linkedlist_clear(struct linkedlist *list)
{
    if (list) {
        while (list->size != 0)
            linkedlist_delete_tail(list, NULL);
    }
}

void *linkedlist_iterator_data(const struct linkedlist_iterator *it)
{
    if (it == NULL || it->node == it->head)
        return NULL;

    return list_entry(it->node, struct linkedlist_node, node)->data;
}

void linkedlist_iterator_init(struct linkedlist *list, struct linkedlist_iterator *it)
{
    if (it == NULL)
        return;

    if (list == NULL) {
        memset(it, 0, sizeof(*it));
    } else {
        it->head = &list->head;
        it->node = &list->head;
    }
}

void *linkedlist_iterator_move(struct linkedlist_iterator *it, const int offset)
{
    unsigned int i, _offset;
    unsigned char flag;

    if (it == NULL)
        return NULL;

    flag = (offset < 0);
    _offset = flag ? -offset : offset;
    for (i = 0; i < _offset; ++i) {
        it->node = flag ? it->node->next : it->node->prev;
        if (it->node == it->head)
            break;
    }

    return linkedlist_iterator_data(it);
}
