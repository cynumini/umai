#ifndef UI_H
#define UI_H

#include <raylib.h>

#include "arena.h"
#include "list.h"

struct Context
{
    struct Node *current;
    Arena *arena;
};
typedef struct Context Context;

Context context_create(void);
void context_destroy(Context *context);

// Node
typedef struct Node Node;

LIST_DEFINE(ListNode, list_node, Node *)

struct Node
{
    Color color;
    Node *parent;
    ListNode *children;
};

void node_open(Context *ctx, Node node);
void node_close(Context *ctx);
void node_add_child(Context *ctx, Node *self, Node *child);

#define NODE(CONTEXT, ...)                                                                                             \
    for (size_t i = (node_open(CONTEXT, (Node)__VA_ARGS__), 0); i < 1; i = 1, node_close(CONTEXT))

#endif /* end of include guard: UI_H */
