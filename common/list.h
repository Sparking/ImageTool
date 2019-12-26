#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "compilers.h"

#undef offsetof
#define offsetof(TYPE, MEMBER) ((size_t)&((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member)                         \
    ((type *)((char *)(ptr) - offsetof(type, member)))

struct list_head {
    struct list_head *prev;
    struct list_head *next;
};

#define LIST_HEAD_INIT(name)    { &(name), &(name) }
#define LIST_HEAD(name)         struct list_head name = LIST_HEAD_INIT(name)
#define INIT_LIST_HEAD(node)                                    \
    do {                                                        \
        (node).prev = &(node);                                  \
        (node).next = &(node);                                  \
    } while (0)

#define list_entry(ptr, type, member)                           \
    container_of(ptr, type, member)

#define list_for_each_entry(pos, type, head, member)                  \
    for (pos = list_entry((head)->next, type, member);  \
         &pos->member != (head);                                \
         pos = list_entry(pos->member.next, type, member))

#define list_for_each_entry_safe(pos, n, type, head, member)          \
    for (pos = list_entry((head)->next, type, member),  \
        n = list_entry(pos->member.next, type, member); \
         &pos->member != (head);                                \
         pos = n, n = list_entry(n->member.next, type, member))

INLINE int list_empty(const struct list_head *head)
{
    return head->next == head;
}

INLINE void __list_add(struct list_head *_node,
                  struct list_head *prev,
                  struct list_head *next)
{
    next->prev = _node;
    _node->next = next;
    _node->prev = prev;
    prev->next = _node;
}

INLINE void list_add(struct list_head *node, struct list_head *head)
{
    __list_add(node, head, head->next);
}

INLINE void list_add_tail(struct list_head *_node, struct list_head *head)
{
    __list_add(_node, head->prev, head);
}

INLINE void __list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

#define LIST_POISON1  ((void *)0x00100100)
#define LIST_POISON2  ((void *)0x00200200)

INLINE void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = (struct list_head*)LIST_POISON1;
    entry->prev = (struct list_head*)LIST_POISON2;
}

#ifdef __cplusplus
}
#endif
