#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "maths.h"

#include "list.h"

struct dotcode_point {
    struct list_head n45;    /**表头:struct dotcode_line_node -> pt(45°线)**/
    struct list_head n135;   /**表头:struct dotcode_line_node -> pt(135°线)**/
    struct point pt;
};

struct dotcode_line_node {
    int index;
    struct list_head node;  /**表头:struct list_head(45°线/135°线)**/
    struct list_head pt;    /**数据:struct dotcode_point -> node45/node135**/
};

struct dotcode_line {
    struct list_head line45;
    struct list_head line135;
};

struct dotcode_point *dotcode_create_new_point(const int x, const int y)
{
    struct dotcode_point *pt;

    pt = (struct dotcode_point *)malloc(sizeof(struct dotcode_point));
    if (pt != NULL) {
        pt->pt.x = x;
        pt->pt.y = y;
        pt->n45.prev = NULL;
        pt->n45.next = NULL;
        pt->n135.prev = NULL;
        pt->n135.next = NULL;
    }

    return pt;
}

struct dotcode_line_node *dotcode_create_new_line_node(const int index)
{
    struct dotcode_line_node *pln;

    pln = (struct dotcode_line_node *)malloc(sizeof(struct dotcode_line_node));
    if (pln != NULL) {
        pln->index = index;
        pln->node.prev = NULL;
        pln->node.next = NULL;
        INIT_LIST_HEAD(pln->pt);
    }

    return pln;
}

void dotcode_line_node_update_index(const struct list_head *head,
    const struct dotcode_line_node *line)
{
    struct list_head *pn;
    struct dotcode_line_node *pln;

    for (pn = head->next; pn != head; pn = pn->next) {
        pln = list_entry(pn, struct dotcode_line_node, node);
        ++pln->index;
    }

    /*debug*/
    int last_index;
    pn = head->next;
    if (pn != head) {
        last_index = list_entry(pn, struct dotcode_line_node, node)->index;
    }
    for (pn = pn->next; pn != head; pn = pn->next) {
        assert(list_entry(pn, struct dotcode_line_node, node)->index != last_index);
        last_index = list_entry(pn, struct dotcode_line_node, node)->index;
    }
}

bool dotcode_insert_line(struct dotcode_line *head, struct dotcode_line_node *line,
        const int dir)
{
    struct list_head *pn, *pl;
    struct dotcode_line_node *pln;

    if (head == NULL || line == NULL)
        return false;

    pl = dir ? &head->line135 : &head->line45;
    for (pn = pl->next; pn != pl; pn = pn->next) {
        pln = list_entry(pn, struct dotcode_line_node, node);
        if (pln->index >= line->index) {
            break;
        }
    }

    line->node.next = line->node.prev = NULL;
    list_add_tail(pn, &line->node);
    dotcode_line_node_update_index(pl, line);

    return true;
}

int main(void)
{
    struct dotcode_line dl;
    struct dotcode_line_node *pln;
    struct dotcode_point *pt;

    pln = dotcode_create_new_line_node(0);
    if (pln == NULL)
        return -1;

    pt = dotcode_create_new_point(0, 0);
    if (pt == NULL) {
        free(pt);
        return -1;
    }

    dotcode_line_insert(&dl, pln, 0);

    free(pt);
    free(pln);

    return 0;
}
