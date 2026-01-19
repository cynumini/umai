#ifndef UI_H
#define UI_H

#include "utils.h"
#include <raylib.h>

// Size & Sizing
typedef enum SizeType
{
    SIZE_TYPE_FIXED,
    SIZE_TYPE_GROW,
    SIZE_TYPE_FIT,
} SizeType;

typedef struct Size
{
    SizeType type;
    int value;
} Size;

Size size_fixed(int value);
Size size_grow(void);
Size size_fit(void);

typedef struct Sizing
{
    Size width;
    Size height;
} Sizing;

// Padding
typedef struct Padding
{
    int top;
    int left;
    int bottom;
    int right;
} Padding;

Padding padding_all(int value);

// Node
typedef struct Node Node;

typedef void (*NodeDraw)(Node *node);

struct Node
{
    Sizing sizing;
    Vector2 position;
    Vector2 actual_position;
    Padding padding;
    NodeDraw draw;
};

// NodeLabel
typedef struct NodeLabel
{
    Node node;
    const char *text;
} NodeLabel;

NodeLabel node_label_init(const char *name);

// NodeButton
typedef struct NodeButton
{
    Node node;
    const char *text;
} NodeButton;

NodeButton node_button_init(const char *name);

// NodeRectangle
typedef struct NodeRectangle
{
    Node node;
} NodeRectangle;

// NodeContainer
typedef enum LayoutDirection
{
    LAYOUT_DIRECTION_LEFT_TO_RIGHT,
    LAYOUT_DIRECTION_TOP_TO_BOTTOM,
} NodeContainerType;

DYNAMIC_ARRAY_INIT(Node *, NodePointerArray);

typedef struct NodeContainer
{
    Node node;
    NodeContainerType type;
    int child_gap;
    NodePointerArray children;
} NodeContainer;

// Don't forget to deinit!
NodeContainer node_container_init(Sizing sizing, NodeContainerType type, Padding padding, int child_gap);
void node_container_deinit(NodeContainer *self);
void node_container_add_child(NodeContainer *self, Node *child);

#endif /* end of include guard: UI_H */
