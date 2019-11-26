#pragma once

#include "linkedlist.h"

struct circular_queue {
    unsigned int size;          /* 数据块的个数 */
    unsigned int block_size;    /* 数据块的大小 */
    unsigned int used_size;     /* 已被使用的数据块个数 */
    unsigned int front;         /* 头指针 */
    unsigned int rear;          /* 尾指针 */
    char data[0];               /* 数据块 */
};

/**
 * @brief circular_queue_create 创建一个指定大小的循环队列
 * @param size 数据块的个数
 * @param block_size 数据块的大小
 * @return 返回循环队列指针
 */
extern struct circular_queue *circular_queue_create(const unsigned int size, const unsigned int block_size);

/**
 * @brief circular_queue_used_size 检查队列已使用的大小
 */
extern unsigned int circular_queue_used_size(const struct circular_queue *q);

/**
 * @brief circular_queue_avaiable_size 检查队列可用的剩余空间大小
 */
extern unsigned int circular_queue_avaiable_size(const struct circular_queue *q);

/**
 * @brief circular_queue_full 判断队列是否已满
 */
extern int circular_queue_full(const struct circular_queue *q);

/**
 * @brief circular_queue_empty 判断队列是否为空
 */
extern int circular_queue_empty(const struct circular_queue *q);

/**
 * @brief circular_queue_enque 将数据入队
 * @param q 循环队列
 * @param data 连续的数据首地址
 * @param size 入队数据个数
 * @return 返回入队的数据个数
 */
extern unsigned int circular_queue_enque(struct circular_queue *q, const void *data, const unsigned int size);

/**
 * @brief circular_queue_enque 将数据出队
 * @param q 循环队列
 * @param data 接受数据的地址
 * @param size 出队的数据个数
 * @return 返回出队的数据个数
 */
extern unsigned int circular_queue_deque(struct circular_queue *q, void *data, const unsigned int size);

/**
 * @brief circular_queue_front 只读取队列头的数据, 不出队该数据
 * @param q 循环队列
 * @param data 接受数据的地址
 * @param size 数据个数
 * @return 失败返回0
 */
extern int circular_queue_front(const struct circular_queue *q, void *data, const unsigned int size);

/**
 * @brief circular_queue_rear 只读取队列尾的数据, 不出对该数据
 * @return 失败返回0
 */
extern int circular_queue_rear(const struct  circular_queue *q, void *data);

/**
 * @brief circular_queue_clear 清空整个队列
 */
extern void circular_queue_clear(struct circular_queue *q);

/**
 * @brief circular_queue_free 释放队列占用的内存
 */
extern void circular_queue_free(struct circular_queue *q);

/* 循环双链表 */
struct linkedlist_queue {
    struct linkedlist q;
};

/* 循环双链表迭代器 */
struct linkedlist_queue_iterator {
    struct linkedlist_iterator it;
};

/**
 * @brief linkedlist_queue_init 循环双链表初始化
 * @param q 链表
 * @param block_size 链表中每个数据块的大小
 */
extern void linkedlist_queue_init(struct linkedlist_queue *q, const unsigned int block_size);

/**
 * @brief linkedlist_queue_size 返回链表中元素的格式, 即双链表的大小
 */
extern unsigned int linkedlist_queue_size(const struct linkedlist_queue *q);

/**
 * @brief linkedlist_queue_enque 入队操作
 * @param q 链表
 * @param data 连续存放的数据首地址
 * @param size 入队的数据个数
 * @return 返回已入队的数据个数
 */
extern unsigned int linkedlist_queue_enque(struct linkedlist_queue *q, const void *data, const unsigned int size);

/**
 * @brief linkedlist_queue_deque 出队操作
 * @param q 链表
 * @param data 接收连续数据的首地址, 传入NULL表示只出队, 但是存放数据
 * @param size 出队的数据个数
 * @return 返回已出队的数据个数
 */
extern unsigned int linkedlist_queue_deque(struct linkedlist_queue *q, void *data, const unsigned int size);

/**
 * @brief linkedlist_queue_front 读取队列头的元素, 但不出队
 * @param q 链表
 * @param data 接收连续数据的首地址
 * @return 返回已读取的数据个数
 */
extern unsigned int linkedlist_queue_front(const struct linkedlist_queue *q, void *data);

/**
 * @brief linkedlist_queue_rear 访问队列尾部的最后一个元素
 * @param q 链表
 * @param data 接收数据的首地址
 * @return 返回已读取的数据个数
 */
extern unsigned int linkedlist_queue_rear(const struct linkedlist_queue *q, void *data);

/**
 * @brief linkedlist_queue_clear 清空队列
 * @param q 链表
 */
extern void linkedlist_queue_clear(struct linkedlist_queue *q);

/**
 * @brief linkedlist_queue_iterator_init 队列迭代器初始化
 * @param p 链表
 * @param it 与链表关联的迭代器
 * @return 返回链表头的第一个元素的地址, 如果链表为空, 则返回NULL
 */
extern void *linkedlist_queue_iterator_init(struct linkedlist_queue *q, struct linkedlist_queue_iterator *it);

/**
 * @brief linkedlist_queue_iterator_move 向后移动队列迭代器
 * @param it 与链表关联的迭代器
 * @return 返回移动后迭代器指向的元素地址, 如果链表为空或者移动到链表尾部, 则返回NULL
 */
extern void *linkedlist_queue_iterator_move(struct linkedlist_queue_iterator *it, const unsigned int offset);

/**
 * @brief linkedlist_queue_iterator_data 返回迭代器指向的元素地址
 * @param it 与链表关联的迭代器
 * @return 返回迭代器指向的元素地址, 如果链表为空, 则返回NULL
 */
extern void *linkedlist_queue_iterator_data(const struct linkedlist_queue_iterator *it);
