#include <stdlib.h>
#include "linkedlist.h"
#include "stack.h"

void stack_init(struct stack *s, const unsigned int block_size)
{
    if (s != NULL) {
        linkedlist_init(&s->s, block_size);
    }
}

void *stack_top(const struct stack *s)
{
    if (stack_empty(s))
        return NULL;

    return linkedlist_tail(&s->s);
}

unsigned int stack_push(struct stack *s, const void *data)
{
    if (s == NULL)
        return 0;

    return linkedlist_add_tail(&s->s, data, 1);
}

unsigned int stack_pop(struct stack *s, void *data)
{
    if (s == NULL)
        return 0;

    return linkedlist_delete_tail(&s->s, data);
}

void stack_clear(struct stack *s)
{
    if (s)
        linkedlist_clear(&s->s);
}
