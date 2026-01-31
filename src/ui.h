#ifndef UI_H
#define UI_H

#include <SKN/arena.h>
#include <raylib.h>

typedef struct UI UI;
typedef struct Paddings Paddings;
typedef struct Sides Sides;
typedef struct Size Size;
typedef struct Node Node;
typedef struct Contrainer Contrainer;
typedef struct ContainerOptions ContainerOptions;
typedef struct Text Text;

struct UI
{
    Arena arena;
    Node *current;
};

void ui_init(void);
void ui_deinit(void);

typedef enum Alignment
{
    ALIGNMENT_LEFT_TOP,
    ALIGNMENT_LEFT_CENTER,
    ALIGNMENT_LEFT_BOTTOM,
    ALIGNMENT_CENTER_TOP,
    ALIGNMENT_CENTER,
    ALIGNMENT_CENTER_BOTTOM,
    ALIGNMENT_RIGHT_TOP,
    ALIGNMENT_RIGHT_CENTER,
    ALIGNMENT_RIGHT_BOTTOM
} Alignment;

struct Sides
{
    u32 top;
    u32 left;
    u32 bottom;
    u32 right;
};

Paddings sides_all(int value);

typedef enum SizeType
{
    SIZE_TYPE_FIT,
    SIZE_TYPE_GROW,
    SIZE_TYPE_FIXED
} SizeType;

struct Size
{
    SizeType type;
    u32 value;
};

void size_fit(void);
void size_grow(void);
void size_fixed(u32 value);

struct Node
{
    const char *class;
    const char *id;
    Node *parent;

    Alignment alignment;
    Color background;
    Color border;
    Color foreground;
    Sides margins;
    Sides paddings;
    Size width;
    Size height;
    bool is_hidden;

    void (*fit_width)(Node *self);
    void (*fit_height)(Node *self);
    void (*grow_width)(Node *self);
    void (*grow_height)(Node *self);
    void (*position)(Node *self);
    void (*add_child)(Node *self, Node *child);
    void (*update)(Node *self);
    void (*draw)(Node *self);

    Rectangle rect;
};

void node_add_child(Node *self, Node *child);

typedef enum LayoutDirection
{
    LAYOUT_DIRECTION_LEFT_TO_RIGHT,
    LAYOUT_DIRECTION_TOP_TO_BOTTOM,
} LayoutDirection;

struct Contrainer
{
    Node node;
    LayoutDirection layout_direction;
    i32 child_gap;
    // NodePtrArray children;
};

struct ContainerOptions
{
    const char *id;
};

void container_open(ContainerOptions options);
void container_close(void);

#define CONTRAINER(...)                                                                                                \
    for (usize i = (container_open((ContainerOptions)__VA_ARGS__), 0); i < 1; container_close(), i = 1)

struct Text
{
    Node node;
    char *text;
    i32 size;
};

// TODO: Nodes I will probably need:
// - Button
// - ScrollView
// - ComboBox
// - Table

#endif /* end of include guard: UI_H */
