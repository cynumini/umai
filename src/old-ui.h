#ifndef UI_H
#define UI_H

#include <raylib.h>

#include "utils.h"

// Size
typedef enum SizeType
{
    SIZE_TYPE_FIT,
    SIZE_TYPE_FIXED,
    SIZE_TYPE_GROW
} SizeType;

typedef struct Size
{
    SizeType type;
    int value;
} Size;

Size size_fixed(int value);
Size size_grow(void);
Size size_fit(void);

// Padding
typedef struct Padding
{
    int top;
    int left;
    int bottom;
    int right;
} Padding;

Padding padding_all(int value);

// Alignment
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

// Node
typedef struct OldNode OldNode;

struct OldNode
{
    Vector2 position;
    Size width;
    Size height;
    Color color;
    Rectangle rect;
    Alignment alignment;
    void (*calc_fit_width)(OldNode *node);
    void (*calc_fit_height)(OldNode *node);
    void (*calc_grow_width)(OldNode *node);
    void (*calc_grow_height)(OldNode *node);
    void (*calc_position)(OldNode *node);
    void (*calc_layout)(OldNode *node);
    void (*update)(OldNode *node);
    void (*draw)(const OldNode *node);
};

// Element
typedef enum LayoutDirection
{
    LAYOUT_DIRECTION_LEFT_TO_RIGHT,
    LAYOUT_DIRECTION_TOP_TO_BOTTOM,
} LayoutDirection;

DYNAMIC_ARRAY_INIT(OldNode *, NodePointerArray);

typedef struct Element
{
    OldNode node;
    Padding padding;
    LayoutDirection layout_direction;
    int child_gap;
    NodePointerArray children;
} OldElement;

OldElement element_init(void);
void element_add_child(OldElement *self, OldNode *node);
void element_deinit(OldElement *self);

// Text
DYNAMIC_ARRAY_INIT(const char *, StringArray);

typedef struct Text
{
    OldNode node;
    const char *text;
    StringArray lines;
    bool wrap;
} Text;

Text text_init(const char *text, bool wrap);
void text_deinit(Text *self);

// Table
typedef struct TableData
{
    void *data;
    size_t (*get_row_count)(void *data);
    size_t (*get_column_count)(void *data);
    void (*get_cell)(void *data, size_t row, size_t column, char *buffer);
} TableData;

typedef struct Table Table;

struct Table
{
    OldNode node;
    TableData data;
    int selected_column;
    // Using inside on_row_selected
    void *user_data;
    void (*on_row_selected)(Table *self, int index, void *user_data);
};

Table table_init(TableData data);
void table_deinit(Table *self);

// TextInput
typedef struct TextInput TextInput;

struct TextInput
{
    OldNode node;
    char text[256];
    // Using inside on_text_changed
    void *user_data;
    void (*on_text_changed)(TextInput *self, void *user_data);
};

TextInput text_input_init(void);
void text_input_deinit(TextInput *self);

#endif /* end of include guard: UI_H */
