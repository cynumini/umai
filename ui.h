#ifndef UI_H
#define UI_H

#include "optional.h"

#define NODES_MAX_LEN 512
#define TEXT_MAX_LEN 256

typedef enum
{
    SIZE_TYPE_FIT,
    SIZE_TYPE_GROW,
    SIZE_TYPE_FIXED
} SizeType;

typedef struct
{
    SizeType type;
    u32 size;
} Size;

Size size_fit();
Size size_grow();
Size size_fixed(u32 size);

typedef enum
{
    DIRECTION_LEFT_TO_RIGHT,
    DIRECTION_TOP_TO_BOTTOM,
} Direction;

typedef struct
{
    Size width;
    Size height;
    PaddingsOptional paddings;
    ColorOptional background;
    Direction direction;
    U32Optional child_gap;
} ContainerOptions;

void ui_begin(ContainerOptions options);
void ui_end();

typedef struct
{
    PaddingsOptional paddings;
    U32Optional border_size;
    Size width;
    Size height;
    ColorOptional background;
} ScrollViewOptions;

void ui_scroll_view_begin(i32 *scroll, RenderTexture *viewport, ScrollViewOptions options);
void ui_scroll_view_end();

typedef struct
{
    PaddingsOptional paddings;
    U32Optional border_size;
    U32Optional font_size;
    ColorOptional background;
    ColorOptional foreground;
    bool wrap;
} TextOptions;

void ui_label(const char *text, TextOptions options);

void ui_draw();

#endif
