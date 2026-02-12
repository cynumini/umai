#ifndef UI_H
#define UI_H

#include <SKN/arena.h>
#include <SKN/array.h>
#include <raylib.h>

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

typedef struct Sides
{
    u32 top;
    u32 left;
    u32 bottom;
    u32 right;
} Sides;

Sides sides_all(int value);

typedef enum SizeType
{
    SIZE_TYPE_FIT,
    SIZE_TYPE_GROW,
    SIZE_TYPE_FIXED
} SizeType;

typedef struct Size
{
    SizeType type;
    u32 value;
} Size;

Size size_grow(void);
Size size_fixed(u32 value);

typedef struct Node Node;
typedef struct Contrainer Contrainer;

DEFINE_DYNAMIC_ARRAY(NodePtrArray, Node *);
DEFINE_DYNAMIC_ARRAY(StringArray, char *);

struct Node
{
    const char *class;
    const char *id;
    Contrainer *parent;

    Alignment alignment;
    Color background;
    Color border;
    Color foreground;
    Sides margins;
    Sides paddings;
    Size width;
    Size height;
    bool is_hidden;

    u32 (*fit_width)(Node *self);
    u32 (*fit_height)(Node *self);
    void (*grow_width)(Node *self);
    void (*grow_height)(Node *self);
    void (*position)(Node *self);
    void (*update)(Node *self);
    void (*draw)(Node *self);
    void (*print)(Node *self, usize level);

    Rectangle rect;
};

typedef enum Direction
{
    DIRECTION_LEFT_TO_RIGHT,
    DIRECTION_TOP_TO_BOTTOM,
} Direction;

struct Contrainer
{
    Node node;
    Direction direction;
    i32 child_gap;
    NodePtrArray children;
};

typedef struct ContainerOptions
{
    const char *id;
    Color background;
    Size width;
    Size height;
    Sides paddings;
    i32 child_gap;
    Direction direction;
} ContainerOptions;

void container_open(ContainerOptions options);
void container_close(void);

#define CONTRAINER(...)                                                                                                \
    for (usize i = (container_open((ContainerOptions)__VA_ARGS__), 0); i < 1; container_close(), i = 1)

typedef struct UI
{
    Arena arena;
    Contrainer *current;
} UI;

void ui_init(void);
void ui_deinit(void);
void ui_draw(void);
void ui_resize(void);
void ui_print_tree(void);

typedef struct Text
{
    Node node;
    char *text;
    bool wrap;
    u32 size;
} Text;

typedef struct TextOptions
{
    const char *id;
    char *text;
    bool wrap;
    u32 size;
} TextOptions;

void text_open(TextOptions options);

#define TEXT(...) text_open((TextOptions)__VA_ARGS__)

// TODO: Nodes I will probably need:
// - Button
// - ScrollView
// - ComboBox
// - Table

#endif /* end of include guard: UI_H */
