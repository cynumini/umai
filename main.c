#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include <raylib.h>
#include <stdio.h>
#include <string.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;
static_assert(sizeof(f32) == 4);
static_assert(sizeof(f64) == 8);

typedef size_t usize;
typedef intptr_t isize;

#define NODES_MAX_LEN 512
#define TEXT_MAX_LEN 256

f32 max_f32(f32 a, f32 b) { return a > b ? a : b; }
f32 min_f32(f32 a, f32 b) { return a < b ? a : b; }

typedef struct
{
    u32 top;
    u32 left;
    u32 bottom;
    u32 right;
} Paddings;

static Paddings paddings_all(u32 value)
{
    return (Paddings){.top = value, .left = value, .bottom = value, .right = value};
}

typedef struct
{
    Paddings paddings;
    Color background;
    Color foreground;
    u32 font_size;
    u32 border_size;
    u32 child_gap;
} Style;

static Style current_style = {
    .paddings = {8, 8, 8, 8},
    .background = {0},
    .foreground = {0},
    .font_size = 20,
    .border_size = 0,
    .child_gap = 8,
};

typedef struct
{
    bool has_value;
    u32 value;
} U32Optional;

U32Optional set_u32_optional(u32 value) { return (U32Optional){.has_value = true, .value = value}; }

typedef enum
{
    DIRECTION_LEFT_TO_RIGHT,
    DIRECTION_TOP_TO_BOTTOM,
} Direction;

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

typedef enum
{
    NODE_TYPE_NONE,
    NODE_TYPE_CONTAINER,
    NODE_TYPE_SCROLL_VIEW,
    NODE_TYPE_LABEL,
} NodeType;

typedef struct
{
    NodeType type;
    usize parent_index;
    Size width;
    Size height;
    Paddings paddings;
    Color background;
    Color foreground;
    Direction direction;
    char text[TEXT_MAX_LEN];
    u32 font_size;
    bool wrap;
    RenderTexture *viewport;
    usize children_len;
    u32 border_size;
    union {
        usize children[NODES_MAX_LEN];
        usize child;
    };
    i32 *scroll;
    u32 child_gap;
    Rectangle rectangle;
} Node;

// static Size size_fit() { return (Size){0}; }
static Size size_grow() { return (Size){.type = SIZE_TYPE_GROW}; }
static Size size_fixed(u32 size) { return (Size){.type = SIZE_TYPE_FIXED, .size = size}; }

typedef struct
{
    bool has_value;
    Color value;
} ColorOptional;

ColorOptional set_color(Color value) { return (ColorOptional){.has_value = true, .value = value}; }

typedef struct
{
    bool has_value;
    Paddings value;
} PaddingsOptional;

PaddingsOptional set_paddings_all(u32 value)
{
    return (PaddingsOptional){.has_value = true, .value = paddings_all(value)};
}

PaddingsOptional set_paddings(Paddings value) { return (PaddingsOptional){.has_value = true, .value = value}; }

typedef struct
{
    Size width;
    Size height;
    PaddingsOptional paddings;
    ColorOptional background;
    Direction direction;
    U32Optional child_gap;
} ContainerOptions;

static Node nodes[NODES_MAX_LEN];
static usize current_container_index = 0;
static usize current_index = 0;

static usize add_node(Node node)
{
    assert(node.type != NODE_TYPE_NONE);
    assert((current_index + 1) < NODES_MAX_LEN);
    nodes[current_index] = node;
    nodes[current_index].parent_index = current_container_index;
    return current_index++;
}

static void ui_begin(ContainerOptions options)
{
    current_container_index = add_node((Node){
        .type = NODE_TYPE_CONTAINER,
        .width = options.width,
        .height = options.height,
        .paddings = options.paddings.has_value ? options.paddings.value : current_style.paddings,
        .background = options.background.has_value ? options.background.value : current_style.background,
        .direction = options.direction,
        .child_gap = options.child_gap.has_value ? options.child_gap.value : current_style.child_gap,
    });
}

typedef enum
{
    AXIS_WIDHT,
    AXIS_HEIGHT,
} Axis;

static u32 node_calc_own_size(Node self, Axis axis)
{
    switch (axis)
    {
    case AXIS_WIDHT:
        return self.paddings.left + self.paddings.right + (self.border_size * 2);
    case AXIS_HEIGHT:
        return self.paddings.top + self.paddings.bottom + (self.border_size * 2);
    }
    unreachable();
    return 0;
}

static void fit_width(Node *self)
{
    f32 own_width = (f32)node_calc_own_size(*self, AXIS_WIDHT);
    switch (self->type)
    {
    case NODE_TYPE_NONE:
        unreachable();
        break;
    case NODE_TYPE_LABEL:
        if (self->wrap)
        {
            unreachable();
        }
        else
        {
            self->rectangle.width = (f32)MeasureText(self->text, (i32)self->font_size) + own_width;
            return;
        }
        break;
    case NODE_TYPE_CONTAINER: {
        for (usize i = 0; i < self->children_len; i++)
        {
            Node *child = &nodes[self->children[i]];
            fit_width(child);
        }
        if (self->width.type == SIZE_TYPE_FIXED)
        {
            self->rectangle.width = (f32)self->width.size;
            return;
        }
        switch (self->direction)
        {
        case DIRECTION_LEFT_TO_RIGHT: {
            f32 width = own_width;
            if (self->children_len > 0)
            {
                width += (f32)((self->children_len - 1) * self->children_len);
            }
            for (usize i = 0; i < self->children_len; i++)
            {
                Node *child = &nodes[self->children[i]];
                width += child->rectangle.width;
            }
            self->rectangle.width = width;
            break;
        }
        case DIRECTION_TOP_TO_BOTTOM: {
            f32 width = 0;
            for (usize i = 0; i < self->children_len; i++)
            {
                Node *child = &nodes[self->children[i]];
                width = max_f32(width, child->rectangle.width);
            }
            self->rectangle.width = width + own_width;
            break;
        }
        }
        return;
    }
    case NODE_TYPE_SCROLL_VIEW:
        if (self->width.type == SIZE_TYPE_FIXED)
        {
            self->rectangle.width = (f32)self->width.size;
        }
        else
        {
            self->rectangle.width = own_width;
        }
        return;
    }
}

static void fit_height(Node *self)
{
    f32 own_height = (f32)node_calc_own_size(*self, AXIS_HEIGHT);
    switch (self->type)
    {
    case NODE_TYPE_NONE:
        unreachable();
        break;
    case NODE_TYPE_LABEL:
        if (self->wrap)
        {
            unreachable();
        }
        else
        {
            self->rectangle.height = (f32)self->font_size + own_height;
        }
        return;
    case NODE_TYPE_CONTAINER: {
        for (usize i = 0; i < self->children_len; i++)
        {
            Node *child = &nodes[self->children[i]];
            fit_height(child);
        }
        if (self->height.type == SIZE_TYPE_FIXED)
        {
            self->rectangle.height = (f32)self->width.size;
            return;
        }
        switch (self->direction)
        {
        case DIRECTION_TOP_TO_BOTTOM: {
            f32 height = own_height;
            if (self->children_len > 0)
            {
                height += (f32)((self->children_len - 1) * self->children_len);
            }
            for (usize i = 0; i < self->children_len; i++)
            {
                Node *child = &nodes[self->children[i]];
                height += child->rectangle.height;
            }
            self->rectangle.height = height;
            break;
        }
        case DIRECTION_LEFT_TO_RIGHT: {
            f32 height = 0;
            for (usize i = 0; i < self->children_len; i++)
            {
                Node *child = &nodes[self->children[i]];
                height = max_f32(height, child->rectangle.width);
            }
            self->rectangle.height = height + own_height;
            break;
        }
        }
        return;
    }
    case NODE_TYPE_SCROLL_VIEW:
        if (self->height.type == SIZE_TYPE_FIXED)
        {
            self->rectangle.height = (f32)self->height.size;
        }
        else
        {
            self->rectangle.height = own_height;
        }
        return;
    }
}

static void grow_width(Node *root)
{
    f32 remaining_width = root->rectangle.width - (f32)node_calc_own_size(*root, AXIS_WIDHT);
    Node *growable[NODES_MAX_LEN] = {0};
    u32 growable_len = 0;
    if (root->type != NODE_TYPE_CONTAINER) return;

    switch (root->direction)
    {
    case DIRECTION_LEFT_TO_RIGHT: {
        if (root->children_len > 0)
        {
            remaining_width -= (f32)(root->children_len * (root->children_len - 1));
        }
        for (usize i = 0; i < root->children_len; i++)
        {
            Node *child = &nodes[root->children[i]];
            remaining_width -= child->rectangle.width;
            if (child->width.type == SIZE_TYPE_GROW)
            {
                growable[growable_len] = child;
                growable_len++;
            }
        }
        while (remaining_width > 0 && growable_len > 0)
        {
            f32 smallest = growable[0]->rectangle.width;
            f32 second_smallest = INFINITY;
            f32 width_to_add = remaining_width;
            for (usize i = 0; i < growable_len; i++)
            {
                Node *child = growable[i];
                if (child->rectangle.width < smallest)
                {
                    second_smallest = smallest;
                    smallest = child->rectangle.width;
                }
                else if (child->rectangle.width > smallest)
                {
                    second_smallest = min_f32(second_smallest, child->rectangle.width);
                    width_to_add = second_smallest - smallest;
                }
            }
            width_to_add = min_f32(width_to_add, remaining_width / (f32)growable_len);
            if (width_to_add == 0) break;
            for (usize i = 0; i < growable_len; i++)
            {
                Node *child = growable[i];
                if (child->rectangle.width == smallest)
                {
                    child->rectangle.width += width_to_add;
                    remaining_width -= width_to_add;
                }
            }
        }
        break;
    }
    case DIRECTION_TOP_TO_BOTTOM: {
        for (usize i = 0; i < root->children_len; i++)
        {
            Node *child = &nodes[root->children[i]];
            if (child->width.type == SIZE_TYPE_GROW)
            {
                child->rectangle.width = max_f32(remaining_width, child->rectangle.width);
            }
        }
        break;
    }
    }
    for (usize i = 0; i < root->children_len; i++)
    {
        Node *child = &nodes[root->children[i]];
        if (child->type == NODE_TYPE_CONTAINER)
        {
            grow_width(child);
        }
    }
}

// TODO: grow_height

static void ui_end()
{
    usize parent_index = nodes[current_container_index].parent_index;
    // current_container_index == parent_index means it's last ui_end
    if (current_container_index == parent_index)
    {
        assert(current_container_index == 0);
        Node *root = &nodes[current_container_index];
        fit_width(root);
        grow_width(root);
        // TODO: continue
        fit_height(root);
    }
    else
    {
        current_container_index = parent_index;
    }
}

typedef struct
{
    PaddingsOptional paddings;
    U32Optional border_size;
    Size width;
    Size height;
    ColorOptional background;
} ScrollViewOptions;

static void ui_scroll_view_begin(i32 *scroll, RenderTexture *viewport, ScrollViewOptions options)
{
    *scroll -= (i32)(GetMouseWheelMove() * 80.f);
    current_container_index = add_node((Node){
        .type = NODE_TYPE_SCROLL_VIEW,
        .scroll = scroll,
        .viewport = viewport,
        .paddings = options.paddings.has_value ? options.paddings.value : current_style.paddings,
        .border_size = options.border_size.has_value ? options.border_size.value : current_style.border_size,
        .width = options.width,
        .height = options.height,
        .background = options.background.has_value ? options.background.value : current_style.background,
    });
}

static void ui_scroll_view_end() { ui_end(); }

typedef struct
{
    PaddingsOptional paddings;
    U32Optional border_size;
    U32Optional font_size;
    ColorOptional background;
    ColorOptional foreground;
    bool wrap;
} TextOptions;

static void ui_label(const char *text, TextOptions options)
{
    usize id = add_node((Node){
        .type = NODE_TYPE_LABEL,
        .wrap = options.wrap,
        .font_size = options.font_size.has_value ? options.font_size.value : current_style.font_size,
        .paddings = options.paddings.has_value ? options.paddings.value : current_style.paddings,
        .background = options.background.has_value ? options.background.value : current_style.background,
        .foreground = options.foreground.has_value ? options.foreground.value : current_style.foreground,
        .border_size = options.border_size.has_value ? options.border_size.value : current_style.border_size,
    });
    assert(strlen(text) < TEXT_MAX_LEN);
    strcpy(nodes[id].text, text);
}

bool is_same_color(Color self, Color other)
{
    return self.r == other.r && self.g == other.g && self.b == other.b && self.a == other.a;
}

// TODO: discard out of screen
static void node_draw(Node self)
{
    switch (self.type)
    {
    case NODE_TYPE_NONE:
        unreachable();
        break;
    case NODE_TYPE_CONTAINER: {
        if (!is_same_color(self.background, BLANK))
        {
            DrawRectangleRec(self.rectangle, self.background);
        }
        for (usize i = 0; i < self.children_len; i++)
        {
            Node child = nodes[self.children[i]];
            node_draw(child);
        }
        break;
    }
    case NODE_TYPE_LABEL: {
        Vector2 offset = {
            self.rectangle.x + (f32)self.paddings.left,
            self.rectangle.y + (f32)self.paddings.top,
        };
        DrawText(self.text, (i32)offset.x, (i32)offset.y, (i32)self.font_size, self.foreground);
        break;
    }
    case NODE_TYPE_SCROLL_VIEW:
        f32 diff = (f32)self.viewport->texture.height - self.rectangle.height;
        DrawTextureRec(self.viewport->texture,
                       (Rectangle){
                           0,
                           diff,
                           self.rectangle.width,
                           -self.rectangle.height,
                       },
                       (Vector2){self.rectangle.x, self.rectangle.y}, WHITE);
        break;
    }
}

static void ui_draw()
{
    assert(current_index > 0);
    // draw
    node_draw(nodes[0]);
    // clear
    current_index = 0;
}

i32 main()
{
    u32 width = 1280, height = 720;
    i32 scroll = 0;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow((i32)width, (i32)height, "umai");
    RenderTexture v = LoadRenderTexture(0, 0);

    while (!WindowShouldClose())
    {
        // update
        if (IsWindowResized())
        {
            width = (u32)GetScreenWidth();
            height = (u32)GetScreenHeight();
        }
        ui_begin((ContainerOptions){
            .background = set_color(RED),
            .width = size_fixed(width),
            .height = size_fixed(height),
        });
        {
            ui_begin((ContainerOptions){
                .background = set_color(GREEN),
                .width = size_grow(),
                .height = size_grow(),
            });
            {
                ui_label("Yes, I am!", (TextOptions){});
                ui_begin((ContainerOptions){
                    .background = set_color(BLUE),
                    .width = size_fixed(100),
                    .height = size_fixed(100),
                });
                ui_end();
                ui_scroll_view_begin(&scroll, &v,
                                     (ScrollViewOptions){
                                         .width = size_grow(),
                                         .height = size_grow(),
                                     });
                {
                    ui_begin((ContainerOptions){});
                    for (usize i = 0; i < 500; i++)
                    {
                        static char buffer[TEXT_MAX_LEN];
                        i32 r = snprintf(buffer, TEXT_MAX_LEN, "Hello, number %zu!", i + 1);
                        assert(r < TEXT_MAX_LEN);
                        ui_label(buffer, (TextOptions){});
                    }
                    ui_end();
                }
                ui_scroll_view_end();
            }
            ui_end();
        }
        ui_end();
        // draw
        BeginDrawing();
        ClearBackground(WHITE);
        ui_draw();
        EndDrawing();
    }
    UnloadRenderTexture(v);
    CloseWindow();
    return 0;
}
