#ifndef UI_H
#define UI_H

#include <raylib.h>

#include <SKN/arena.h>
#include <SKN/list.h>

struct Context
{
    struct Node *current;
    Arena *arena;
};
typedef struct Context Context;

Context context_create(Arena *arena);
void context_draw(Context *ctx);
void context_calc_layout(Context *ctx);

// Padding
typedef struct Padding
{
    int top;
    int left;
    int bottom;
    int right;
} Padding;

Padding padding_all(int value);

// Size
typedef enum SizeType
{
    SIZE_TYPE_FIT,
    SIZE_TYPE_GROW,
    SIZE_TYPE_FIXED,
} SizeType;

typedef struct Size
{
    SizeType type;
    float value;
} Size;

Size size_fit(void);
Size size_grow(void);
Size size_fixed(float value);

// Node
typedef struct Node Node;

LIST_DEFINE(Node *, NodePtrList);

struct Node
{
    const char *id;
    Color color;
    Padding padding;
    float child_gap;
    Size width;
    Size height;
    Rectangle rect;
    Node *parent;
    NodePtrList children;
};

void node_open(Context *ctx, Node node);
void node_close(Context *ctx);
void node_print(Node *node, int level);
void node_draw(Node *node);
void node_calc_layout(Context *ctx, Node *node);

#define NODE(CONTEXT, ...)                                                                                             \
    for (size_t i = (node_open(CONTEXT, (Node)__VA_ARGS__), 0); i < 1; i = 1, node_close(CONTEXT))

#endif /* end of include guard: UI_H */
