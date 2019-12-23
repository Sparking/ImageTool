#pragma once

#include "linkedlist.h"

struct stack {
    struct linkedlist s;
};

/**
 * @brief stack_init 栈初始化
 * @param s 栈
 * @param block_size 栈中每个元素的大小
 */
extern void stack_init(struct stack *s, const unsigned int block_size);

/**
 * @brief stack_empty 判断栈是否为空
 */
INLINE unsigned char stack_empty(const struct stack *s)
{
    if (s == NULL)
        return 1;

    return s->s.size == 0;
}

/**
 * @brief stack_top 返回栈顶元素的地址
 * @return 空栈返回NULL, 否则返回栈顶元素的地址
 */
extern void *stack_top(const struct stack *s);

/**
 * @brief stack_push 将一个元素入栈
 * @param data 入栈的元素地址
 * @return 成功返回入栈的元素个数, 即1。失败返回0, 表示申请内存失败
 */
extern unsigned int stack_push(struct stack *s, const void *data);

/**
 * @brief stack_pop 将栈顶的元素出栈
 * @param data 元素出栈的地址。设置为NULL, 表示元素出栈但不保存
 * @return 成功返回出栈的元素个数, 即1。失败返回0, 表示栈是空的
 */
extern unsigned int stack_pop(struct stack *s, void *data);

/**
 * @brief stack_clear 清空栈
 */
extern void stack_clear(struct stack *s);
