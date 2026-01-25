#ifndef UI_H
#define UI_H

#include <raylib.h>

#include "SKN/arena.h"

struct Context
{
    struct Node *current;
    Arena *arena;
};
typedef struct Context Context;

Context context_create(Arena *arena);

// Node
typedef struct Node Node;
typedef struct NodeChildren NodeChildren;

struct Node
{
    const char *id;
    Color color;
    Node *parent;
    NodeChildren *children;
};

struct NodeChildren
{
    Node *value;
    NodeChildren *next;
};

void node_open(Context *ctx, Node node);
void node_close(Context *ctx);
void node_add_child(Context *ctx, Node *self, Node *child);
void node_print(Node *node, int level);

#define NODE(CONTEXT, ...)                                                                                             \
    for (size_t i = (node_open(CONTEXT, (Node)__VA_ARGS__), 0); i < 1; i = 1, node_close(CONTEXT))

#endif /* end of include guard: UI_H */
