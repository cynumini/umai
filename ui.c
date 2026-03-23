#include "ui.h"

#include <assert.h>
#include <math.h>
#include <string.h>

#include <raylib.h>

#include "math.h"
#include "sakana.h"

typedef enum
{
    ALIGNMENT_LEFT_TOP,
    ALIGNMENT_LEFT_CENTER,
    ALIGNMENT_LEFT_BOTTOM,
    ALIGNMENT_CENTER_TOP,
    ALIGNMENT_CENTER,
    ALIGNMENT_CENTER_BOTTOM,
    ALIGNMENT_RIGHT_TOP,
    ALIGNMENT_RIGHT_CENTER,
    ALIGNMENT_RIGHT_BOTTOM,
} Alignment;

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
    .foreground = {0, 0, 0, 255},
    .font_size = 20,
    .border_size = 0,
    .child_gap = 8,
};

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
    Alignment alignment;
} Node;

Size size_fit() { return (Size){0}; }
Size size_grow() { return (Size){.type = SIZE_TYPE_GROW}; }
Size size_fixed(u32 size) { return (Size){.type = SIZE_TYPE_FIXED, .size = size}; }

static Node nodes[NODES_MAX_LEN];
static usize current_container_index = 0;
static usize current_index = 0;

static usize add_node(Node node)
{
    assert(node.type != NODE_TYPE_NONE);
    assert((current_index + 1) < NODES_MAX_LEN);
    nodes[current_index] = node;
    nodes[current_index].parent_index = current_container_index;
    if (current_index != 0)
    {
        Node *parent = &nodes[current_container_index];
        if (parent->type == NODE_TYPE_CONTAINER)
        {
            parent->children[parent->children_len] = current_index;
            parent->children_len++;
        }
        else
        {
            parent->child = current_index;
        }
    }
    return current_index++;
}

void ui_begin(ContainerOptions options)
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
        }
        return;
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
                width = f32_max(width, child->rectangle.width);
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
        if (self->child != 0)
        {
            fit_width(&nodes[self->child]);
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
            self->rectangle.height = (f32)self->height.size;
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
                height = f32_max(height, child->rectangle.height);
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
        }        else
        {
            self->rectangle.height = own_height;
        }
        if (self->child != 0)
        {
            fit_height(&nodes[self->child]);
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
                    second_smallest = f32_min(second_smallest, child->rectangle.width);
                    width_to_add = second_smallest - smallest;
                }
            }
            width_to_add = f32_min(width_to_add, remaining_width / (f32)growable_len);
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
                child->rectangle.width = f32_max(remaining_width, child->rectangle.width);
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

static void grow_height(Node *root)
{
    f32 remaining_height = root->rectangle.height - (f32)node_calc_own_size(*root, AXIS_HEIGHT);
    Node *growable[NODES_MAX_LEN] = {0};
    u32 growable_len = 0;
    if (root->type != NODE_TYPE_CONTAINER) return;

    switch (root->direction)
    {
    case DIRECTION_TOP_TO_BOTTOM: {
        if (root->children_len > 0)
        {
            remaining_height -= (f32)(root->children_len * (root->children_len - 1));
        }
        for (usize i = 0; i < root->children_len; i++)
        {
            Node *child = &nodes[root->children[i]];
            remaining_height -= child->rectangle.height;
            if (child->height.type == SIZE_TYPE_GROW)
            {
                growable[growable_len] = child;
                growable_len++;
            }
        }
        while (remaining_height > 0 && growable_len > 0)
        {
            f32 smallest = growable[0]->rectangle.height;
            f32 second_smallest = INFINITY;
            f32 height_to_add = remaining_height;
            for (usize i = 0; i < growable_len; i++)
            {
                Node *child = growable[i];
                if (child->rectangle.height < smallest)
                {
                    second_smallest = smallest;
                    smallest = child->rectangle.height;
                }
                else if (child->rectangle.height > smallest)
                {
                    second_smallest = f32_min(second_smallest, child->rectangle.height);
                    height_to_add = second_smallest - smallest;
                }
            }
            height_to_add = f32_min(height_to_add, remaining_height / (f32)growable_len);
            if (height_to_add == 0) break;
            for (usize i = 0; i < growable_len; i++)
            {
                Node *child = growable[i];
                if (child->rectangle.height == smallest)
                {
                    child->rectangle.height += height_to_add;
                    remaining_height -= height_to_add;
                }
            }
        }
        break;
    }
    case DIRECTION_LEFT_TO_RIGHT: {

        for (usize i = 0; i < root->children_len; i++)
        {
            Node *child = &nodes[root->children[i]];
            if (child->height.type == SIZE_TYPE_GROW)
            {
                child->rectangle.height = f32_max(remaining_height, child->rectangle.height);
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
            grow_height(child);
        }
    }
}

static void position(Node *root)
{
    f32 left_offset = root->rectangle.x + (f32)(root->paddings.right + root->border_size);
    f32 top_offset = root->rectangle.y + (f32)(root->paddings.top + root->border_size);

    f32 remaining_width = root->rectangle.width;
    f32 remaining_height = root->rectangle.width;

    Node *container;

    if (root->type == NODE_TYPE_CONTAINER)
        container = root;
    else if (root->type == NODE_TYPE_SCROLL_VIEW && root->child != 0 && nodes[root->child].type == NODE_TYPE_CONTAINER)
        container = &nodes[root->child];
    else
        return;

    for (usize i = 0; i < container->children_len; i++)
    {
        Node *child = &nodes[container->children[i]];
        remaining_width -= child->rectangle.width;
        remaining_height -= child->rectangle.height;
    }

    switch (container->direction)
    {
    case DIRECTION_LEFT_TO_RIGHT: {
        if (container->children_len > 0)
        {
            remaining_width = (f32)((container->children_len - 1) * container->child_gap);
        }
        if (container->alignment == ALIGNMENT_CENTER)
        {
            left_offset += remaining_width / 2;
        }
        for (usize i = 0; i < container->children_len; i++)
        {
            Node *child = &nodes[container->children[i]];
            Vector2 child_position = {0};
            child_position.x += left_offset;
            child_position.y += top_offset;
            if (container->alignment == ALIGNMENT_CENTER)
            {
                child_position.y = (root->rectangle.height - child->rectangle.height) / 2;
            }
            child->rectangle.x = child_position.x;
            child->rectangle.y = child_position.y;
            left_offset += child->rectangle.width + (f32)container->child_gap;
        }
        break;
    }
    case DIRECTION_TOP_TO_BOTTOM: {
        if (container->children_len > 0)
        {
            remaining_height = (f32)((container->children_len - 1) * container->child_gap);
        }
        if (container->alignment == ALIGNMENT_CENTER)
        {
            top_offset += remaining_height / 2;
        }
        for (usize i = 0; i < container->children_len; i++)
        {
            Node *child = &nodes[container->children[i]];
            Vector2 child_position = {0};
            if (container->alignment == ALIGNMENT_CENTER)
            {
                child_position.x = (root->rectangle.width - child->rectangle.width) / 2;
            }
            else
            {
                child_position.x += left_offset;
            }
            child_position.y += top_offset;
            child->rectangle.x = child_position.x;
            child->rectangle.y = child_position.y;
            top_offset += child->rectangle.height + (f32)container->child_gap;
        }
        break;
    }
    }

    for (usize i = 0; i < container->children_len; i++)
    {
        Node *child = &nodes[container->children[i]];
        if (child->type == NODE_TYPE_CONTAINER || child->type == NODE_TYPE_SCROLL_VIEW)
        {
            position(child);
        }
    }
}

bool is_same_color(Color self, Color other)
{
    return self.r == other.r && self.g == other.g && self.b == other.b && self.a == other.a;
}

// TODO: discard out of screen
static void node_draw(Node *self)
{
    switch (self->type)
    {
    case NODE_TYPE_NONE:
        unreachable();
        break;
    case NODE_TYPE_CONTAINER: {
        if (!is_same_color(self->background, BLANK))
        {
            DrawRectangleRec(self->rectangle, self->background);
        }
        for (usize i = 0; i < self->children_len; i++)
        {
            Node *child = &nodes[self->children[i]];
            node_draw(child);
        }
        break;
    }
    case NODE_TYPE_LABEL: {
        Vector2 offset = {
            self->rectangle.x + (f32)self->paddings.left,
            self->rectangle.y + (f32)self->paddings.top,
        };
        DrawText(self->text, (i32)offset.x, (i32)offset.y, (i32)self->font_size, self->foreground);
        break;
    }
    case NODE_TYPE_SCROLL_VIEW:
        f32 diff = (f32)self->viewport->texture.height - self->rectangle.height;
        DrawTextureRec(self->viewport->texture,
                       (Rectangle){
                           0,
                           diff,
                           self->rectangle.width,
                           -self->rectangle.height,
                       },
                       (Vector2){self->rectangle.x, self->rectangle.y}, WHITE);
        break;
    }
}

void ui_end()
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
        grow_height(root);
        position(root);

        for (usize i = current_index; i > 0; i--)
        {
            Node *child = &nodes[i - 1];
            if (child->type == NODE_TYPE_SCROLL_VIEW)
            {
                Node *child_child = &nodes[child->child];
                i32 width = (i32)child->rectangle.width;
                i32 height = (i32)child->rectangle.height;

                if (width > child->viewport->texture.width || height > child->viewport->texture.height)
                {
                    UnloadRenderTexture(*child->viewport);
                    *child->viewport = LoadRenderTexture(width, height);
                }
                Vector2 temp = {child->rectangle.x, child->rectangle.y};
                *child->scroll = i32_clamp(*child->scroll, 0, (i32)(root->rectangle.height - child->rectangle.height));
                child_child->rectangle.x = 0;
                child_child->rectangle.y = -(f32)(*child->scroll);
                BeginTextureMode(*child->viewport);
                ClearBackground(WHITE);
                position(child_child);
                node_draw(child_child);
                EndTextureMode();
                child->rectangle.x = temp.x;
                child->rectangle.y = temp.y;
            }
        }
    }
    else
    {
        current_container_index = parent_index;
    }
}

void ui_scroll_view_begin(i32 *scroll, RenderTexture *viewport, ScrollViewOptions options)
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

void ui_scroll_view_end() { ui_end(); }

void ui_label(const char *text, TextOptions options)
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

void ui_draw()
{
    assert(current_index > 0);
    // draw
    node_draw(&nodes[0]);
    // clear
    current_index = 0;
}
