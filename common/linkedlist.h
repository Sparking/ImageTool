#pragma once

#include "list.h"

struct linkedlist {
    struct list_head head;      /* 链表头 */
    unsigned int block_size;    /* 链表中每块数据的大小 */
    unsigned int size;          /* 链表中的元素个数 */
};

struct linkedlist_iterator {
    struct list_head *head;     /* 链表头 */
    struct list_head *node;     /* 对应的链表节点 */
};

/**
 * @brief linkedlist_init 初始化链表
 */
extern void linkedlist_init(struct linkedlist *list, const unsigned int block_size);

/**
 * @brief linkedlist_head 访问链表头的元素
 * @return 链表为空时返回NULL, 否则返回元素对应的地址
 */
extern void *linkedlist_head(const struct linkedlist *list);

/**
 * @brief linkedlist_tail 访问链表的最后一个元素
 * @return 链表为空时返回NULL, 否则返回元素对应的地址
 */
extern void *linkedlist_tail(const struct linkedlist *list);

/**
 * @brief linkedlist_size 返回链表的大小
 */
extern unsigned int linkedlist_size(const struct linkedlist *list);

/**
 * @brief linkedlist_add 向链表头部加入n个元素
 * @note 数据原来的顺序: 1 2 3 4 5 ... n
 *      插入后的顺序: head -> n ... 5 4 3 2 1 -> ...
 */
extern unsigned int linkedlist_add(struct linkedlist *list,
		const void *data, const unsigned int size);

/**
 * @brief linkedlist_add_tail 向链表尾部加入n个元素
 * @note 数据原来的顺序: 1 2 3 4 5 ... n
 *      插入后的顺序: ... -> n ... 5 4 3 2 1 -> head
 */
extern unsigned int linkedlist_add_tail(struct linkedlist *list,
		const void *data, const unsigned int size);

/**
 * @brief linkedlist_delete 删除链表头部第一个元素
 */
extern unsigned int linkedlist_delete(struct linkedlist *list, void *data);

/**
 * @brief linkedlist_delete_tail 删除链表尾部的最后一个元素
 */
extern unsigned int linkedlist_delete_tail(struct linkedlist *list, void *data);

/**
 * @brief linkedlist_clear 清空整个链表
 */
extern void linkedlist_clear(struct linkedlist *list);

/**
 * @brief linkedlist_iterator_init 链表迭代器初始化
 * @note 链表迭代器要和具体的链表进行关联, 迭代器一开始指向链表头, 链表头没有记录数据。
 *       迭代器使用过程中要注意删除元素可能会使迭代器失效。
 */
extern void linkedlist_iterator_init(struct linkedlist *list,
		struct linkedlist_iterator *it);

/**
 * @brief linkedlist_iterator_data 返回迭代器指向的元素的地址
 * @note 迭代器一开始指向链表头, 链表头没有记录数据, 当迭代器移动到链表头时会返回NULL
 */
extern void *linkedlist_iterator_data(const struct linkedlist_iterator *it);

/**
 * @brief linkedlist_iterator_move 移动迭代器
 * @param offset offset大于0时, 迭代器向前移动(从链表头向链表尾移动);
 *               offset小于0时, 迭代器向后移动(从链表尾向链表头移动)
 * @return 返回移动后的迭代器指向的元素地址
 */
extern void *linkedlist_iterator_move(struct linkedlist_iterator *it,
		const int offset);
