#ifndef UI_H
#define UI_H

#include <SKN/arena.h>
#include <SKN/array.h>
#include <raylib.h>

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

Size size_fit(void);
Size size_grow(void);
Size size_fixed(u32 value);

typedef struct Node Node;
typedef struct Container Container;

DEFINE_DYNAMIC_ARRAY(NodePtrArray, Node *);
DEFINE_DYNAMIC_ARRAY(StringArray, char *);

typedef enum NodeType
{
    NODE_TYPE_NODE,
    NODE_TYPE_CONTRAINER,
    NODE_TYPE_TEXT,
} NodeType;

typedef enum Alignment // rev 0
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

typedef struct Sides // rev 0
{
    u32 top;
    u32 left;
    u32 bottom;
    u32 right;
} Sides;

Sides sides_all(int value);

typedef struct Event
{
    bool mouse_hover;
} Event;

struct Node
{
    const char *class;
    const char *id;
    Container *parent;
    NodeType type;
    Arena *arena;

    Alignment alignment;
    Color background;
    u32 border_size;
    Color border_color;
    Color foreground;
    Sides margins;
    Sides paddings;
    Size width;
    Size height;
    bool is_hidden;

    u32 (*fit_width)(Node *self);
    u32 (*fit_height)(Node *self);
    void (*update)(Node *self, Event mouse_position);
    void (*draw)(Node *self);
    void (*print)(Node *self, usize level);

    Rectangle rect;
};

typedef enum Direction
{
    DIRECTION_LEFT_TO_RIGHT,
    DIRECTION_TOP_TO_BOTTOM,
} Direction;

struct Container
{
    Node node;
    Direction direction;
    i32 child_gap;
    NodePtrArray children;
    void (*grow_width)(Container *self);
    void (*grow_height)(Container *self);
    void (*position)(Container *self);
};

#define DEFAULT_NODE_OPTIONS                                                                                           \
    const char *id;                                                                                                    \
    Size width;                                                                                                        \
    Size height

typedef struct ContainerOptions
{
    DEFAULT_NODE_OPTIONS;
    Color background;
    Sides paddings;
    Alignment alignment;
    i32 child_gap;
    Direction direction;
    u32 border_size;
    Color border_color;
} ContainerOptions;

void container_open(ContainerOptions options);
void container_close(void);
Container *container_create(Arena *arena, ContainerOptions options);
void container_add_child(Container *container, Node *child);

#define CONTRAINER(...)                                                                                                \
    for (usize i = (container_open((ContainerOptions)__VA_ARGS__), 0); i < 1; container_close(), i = 1)

typedef struct UI
{
    Arena arena;
    Container *root;
    bool update;
    Vector2 mouse_position;
} UI;

void ui_init(void);
void ui_deinit(void);
void ui_draw(void);
void ui_resize(u32 width, u32 height);
Node *ui_get_by_id(const char *id);
void ui_update(void);
void ui_commit(void);
void ui_print_tree(void);

typedef struct Text
{
    Node node;
    const char *text;
    bool wrap;
    u32 size;
} Text;

DEFINE_DYNAMIC_ARRAY(TextPtrArray, Text *);

typedef struct TextOptions
{
    DEFAULT_NODE_OPTIONS;
    bool wrap;
    u32 size;
} TextOptions;

void text_open(const char *text, TextOptions options);
Text *text_create(Arena *arena, const char *text, TextOptions options);

#define TEXT(TEXT_ARG, ...) text_open(TEXT_ARG, (TextOptions)__VA_ARGS__)

// Button
typedef struct Button Button;

typedef void (*ButtonOnClick)(Button *self);

// typedef enum ButtonState
// {
//     BUTTON_STATE_NORMAL,
//     BUTTON_STATE_HOVER,
// } ButtonState;

struct Button
{
    Container container;
    Text text;

    // ButtonState state;
    void *user_data;
    ButtonOnClick on_click;
};

typedef struct ButtonOptions
{
    DEFAULT_NODE_OPTIONS;
    void *user_data;
    ButtonOnClick on_click;
} ButtonOptions;

Button *button_create(Arena *arena, const char *text, ButtonOptions options);

// Tabs
typedef struct Tab
{
    Container root;
    Text text;
    Button button;

    bool closable;
    bool active;
    usize index;
} Tab;

typedef struct Tabs
{
    Container root;
    Container top;
    Container bottom;
    usize current;
} Tabs;

typedef struct TabsOptions
{
    DEFAULT_NODE_OPTIONS;
} TabsOptions;

void tabs_open(TabsOptions options);

#define TABS(...) for (usize i = (tabs_open((TabsOptions)__VA_ARGS__), 0); i < 1; container_close(), i = 1)

Tabs *tabs_create(Arena *arena, TabsOptions options);

typedef struct TabOptions
{
    bool closable;
} TabOptions;

void tabs_add_tab(Tabs *tabs, const char *name, TabOptions options, Node *tab);

// CheckBox
typedef struct CheckBox CheckBox;

typedef void (*CheckBoxOnClick)(CheckBox *self);

struct CheckBox
{
    Node node;
    bool state;
    void *user_data;
    CheckBoxOnClick on_click;
};

typedef struct CheckBoxOptions
{
    DEFAULT_NODE_OPTIONS;
    void *user_data;
    ButtonOnClick on_click;
} CheckBoxOptions;

CheckBox *check_box_create(Arena *arena, const char *text, CheckBoxOptions options);

extern UI ui;

// TODO: Nodes I will probably need:
// - Button
// - ScrollView
// - ComboBox
// - Table

#endif /* end of include guard: UI_H */
