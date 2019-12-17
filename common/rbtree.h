#pragma once

#include <stdio.h>

#define RB_RED              0
#define RB_BLACK            1
struct rb_node {
    unsigned long   rb_parent_color;
    struct rb_node *rb_left;
    struct rb_node *rb_right;
};

struct rb_root {
    struct rb_node *rb_node;
};

#define rb_parent(rb)       ((struct rb_node *)(((char *)NULL) + (((rb)->rb_parent_color & ~3))))
#define rb_color(rb)        ((rb)->rb_parent_color & 1)
#define rb_is_red(rb)       (!rb_color(rb))
#define rb_is_black(rb)     rb_color(rb)
#define rb_set_red(rb)      do { (rb)->rb_parent_color &= ~1; } while (0)
#define rb_set_black(rb)    do { (rb)->rb_parent_color |= 1; } while(0)

static inline void rb_set_parent(struct rb_node *rb, struct rb_node *p)
{
    rb->rb_parent_color = (unsigned long)(rb_color(rb) | (((char *)p) - ((char *)NULL)));
}

static inline void rb_set_color(struct rb_node *rb, unsigned color)
{
    rb->rb_parent_color = (unsigned long)((rb->rb_parent_color & ~1) | color);
}

static inline void rb_set_parent_color(struct rb_node *rb, struct rb_node *p,
            unsigned color)
{
    rb->rb_parent_color = (unsigned long)((((char *)p) - ((char *)NULL)) | color);
}

#define rb_entry(ptr, type, m)  ((type *)((((char *)(ptr)) - ((char *)&((type *)0)->m))))

#define RB_ROOT             (struct rb_root){NULL,}
#define RB_EMPTY_ROOT(root) ((root)->rb_node == NULL)
#define RB_EMPTY_NODE(rb)   ((rb)->rb_parent_color == (unsigned long)(((char *)(rb)) - ((char *)NULL)))
#define RB_CLEAR_NODE(rb)   ((rb)->rb_parent_color = (unsigned long)(((char *)(rb)) - ((char *)NULL)))

static inline void rb_init_node(struct rb_node *rb)
{
    rb->rb_parent_color = 0;
    rb->rb_right = rb->rb_left = NULL;
    RB_CLEAR_NODE(rb);
}

static inline void rb_link_node(struct rb_node *node, struct rb_node *parent,
        struct rb_node **rb_link)
{
    node->rb_parent_color = (unsigned long)((((char *)parent) - ((char *)NULL)));
    node->rb_left = node->rb_right = NULL;

    *rb_link = node;
}

typedef void (*rb_augment_f)(struct rb_node *node, void *data);

extern void rb_insert_color(struct rb_node *, struct rb_root *);
extern void rb_erase(struct rb_node *, struct rb_root *);

extern void rb_augment_insert(struct rb_node *, rb_augment_f, void *);
extern struct rb_node *rb_augment_erase_begin(struct rb_node *);
extern void rb_augment_erase_end(struct rb_node *, rb_augment_f, void *);

extern struct rb_node *rb_next(const struct rb_node *);
extern struct rb_node *rb_prev(const struct rb_node *);
extern struct rb_node *rb_first(const struct rb_root *);
extern struct rb_node *rb_last(const struct rb_root *);

extern void rb_replace_node(struct rb_node *victim, struct rb_node *parent,
        struct rb_root *root);
