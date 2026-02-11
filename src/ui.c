#include "ui.h"
#include "SKN/arena.h"

#include <math.h>
#include <raylib.h>
#include <stdio.h>

#include <SKN/math.h>
#include <raymath.h>

static UI ui = {0};

Size size_fit(void)
{
    return (Size){SIZE_TYPE_FIT, 0};
}

Size size_grow(void)
{
    return (Size){SIZE_TYPE_GROW, 0};
}

Size size_fixed(u32 value)
{
    return (Size){SIZE_TYPE_FIXED, value};
}

Sides sides_all(int value)
{
    return (Sides){value, value, value, value};
}

static void node_add_child(Node *self, Node *child)
{
    if (self->add_child != NULL)
    {
        self->add_child(self, child);
    }
}

static NodePtrArray *node_get_children(Node *self)
{

    if (self->get_children != NULL)
    {
        return self->get_children(self);
    }
    return NULL;
}

static void node_fit_width(Node *self)
{
    if (self->fit_width != NULL)
    {
        self->rect.width = self->fit_width(self);
    }
}

static void node_grow_width(Node *self)
{
    if (self->grow_width != NULL)
    {
        self->grow_width(self);
    }
}

static void node_fit_height(Node *self)
{
    if (self->fit_height != NULL)
    {
        self->fit_height(self);
    }
}

static void node_grow_height(Node *self)
{
    if (self->grow_height != NULL)
    {
        self->grow_height(self);
    }
}

static void node_position(Node *self)
{
    if (self->position != NULL)
    {
        self->position(self);
    }
}

static void node_print(Node *self, usize level)
{
    for (usize i = 0; i < level; i++)
    {
        printf("\t");
    }
    const char *id = self->id != NULL ? self->id : "NULL";
    printf("id = %s\n", id);
    NodePtrArray *children = node_get_children(self);
    if (children)
    {
        for (usize i = 0; i < children->len; i++)
        {
            node_print(children->items[i], level + 1);
        }
    }
}

static void node_draw(Node *self)
{
    if (self->draw != NULL)
    {
        self->draw(self);
    }
}

DYNAMIC_ARRAY_IMPL_STATIC(NodePtrArray, Node *, node_ptr_array)

static void container_add_child(Node *node, Node *child)
{
    Contrainer *self = (Contrainer *)node;
    child->parent = node;
    node_ptr_array_add(&ui.arena, &self->children, child);
}

static NodePtrArray *container_get_children(Node *node)
{
    Contrainer *self = (Contrainer *)node;
    return &self->children;
}

static u32 container_fit_width(Node *node)
{
    Contrainer *self = (Contrainer *)node;

    for (usize i = 0; i < self->children.len; i++)
    {
        node_fit_width(self->children.items[i]);
    }

    if (node->width.type == SIZE_TYPE_FIXED)
    {
        return node->width.value;
    }

    if (self->direction == DIRECTION_LEFT_TO_RIGHT)
    {
        f32 total_width = node->paddings.left + node->paddings.right;
        if (self->children.len > 0)
        {
            total_width += (self->children.len - 1) * self->child_gap;
        }
        for (usize i = 0; i < self->children.len; i++)
        {
            total_width += self->children.items[i]->rect.width;
        }
        return total_width;
    }
    // DIRECTION_TOP_TO_BOTTOM
    f32 max_width = 0;
    for (usize i = 0; i < self->children.len; i++)
    {
        max_width = MAX(self->children.items[i]->rect.width, max_width);
    }
    return max_width + node->paddings.left + node->paddings.right;
}

static void container_fit_height(Node *node)
{
    Contrainer *self = (Contrainer *)node;

    for (usize i = 0; i < self->children.len; i++)
    {
        node_fit_height(self->children.items[i]);
    }

    if (node->height.type == SIZE_TYPE_FIXED)
    {
        node->rect.height = node->height.value;
        return;
    }

    if (self->direction == DIRECTION_LEFT_TO_RIGHT)
    {
        f32 max_height = 0;
        for (usize i = 0; i < self->children.len; i++)
        {
            max_height = MAX(self->children.items[i]->rect.height, max_height);
        }
        node->rect.height = max_height + node->paddings.top + node->paddings.bottom;
    }
    else
    {
        f32 total_height = node->paddings.top + node->paddings.bottom;
        if (self->children.len > 0)
        {
            total_height += (self->children.len - 1) * self->child_gap;
        }
        for (usize i = 0; i < self->children.len; i++)
        {
            total_height += self->children.items[i]->rect.height;
        }
        node->rect.height = total_height;
    }
}

static void container_grow_width(Node *node)
{
    Contrainer *self = (Contrainer *)node;

    f32 remaining_width = node->rect.width;
    remaining_width -= node->paddings.left + node->paddings.right;
    ArenaSave arena_save = arena_quick_save(&ui.arena);
    NodePtrArray growable = {0};

    if (self->direction == DIRECTION_LEFT_TO_RIGHT)
    {
        if (self->children.len > 0)
        {
            remaining_width -= self->child_gap * (self->children.len - 1);
        }

        for (usize i = 0; i < self->children.len; i++)
        {
            Node *child = self->children.items[i];
            remaining_width -= child->rect.width;
            if (child->width.type == SIZE_TYPE_GROW)
            {
                node_ptr_array_add(&ui.arena, &growable, child);
            }
        }

        while (remaining_width > 0 && growable.len > 0)
        {
            f32 smallest = growable.items[0]->rect.width;
            f32 second_smallest = INFINITY;
            f32 width_to_add = remaining_width;
            for (usize i = 0; i < growable.len; i++)
            {
                Node *child = growable.items[i];
                if (child->rect.width < smallest)
                {
                    second_smallest = smallest;
                    smallest = child->rect.width;
                }
                if (child->rect.width > smallest)
                {
                    second_smallest = MIN(second_smallest, child->rect.width);
                    width_to_add = second_smallest - smallest;
                }
            }
            width_to_add = MIN(width_to_add, remaining_width / growable.len);
            if (width_to_add == 0)
                break;
            for (usize i = 0; i < growable.len; i++)
            {
                Node *child = growable.items[i];
                if (child->rect.width == smallest)
                {
                    child->rect.width += width_to_add;
                    remaining_width -= width_to_add;
                }
            }
        }
    }
    else if (self->direction == DIRECTION_TOP_TO_BOTTOM)
    {
        for (usize i = 0; i < self->children.len; i++)
        {
            Node *child = self->children.items[i];
            if (child->width.type == SIZE_TYPE_GROW)
            {
                child->rect.width = remaining_width;
            }
        }
    }

    arena_quick_load(&ui.arena, arena_save);

    for (usize i = 0; i < self->children.len; i++)
    {
        node_grow_width(self->children.items[i]);
    }
}

static void container_grow_height(Node *node)
{
    Contrainer *self = (Contrainer *)node;
    f32 remaining_height = node->rect.height;
    remaining_height -= node->paddings.top + node->paddings.bottom;
    ArenaSave arena_save = arena_quick_save(&ui.arena);
    NodePtrArray growable = {0};

    if (self->direction == DIRECTION_LEFT_TO_RIGHT)
    {
        for (usize i = 0; i < self->children.len; i++)
        {
            Node *child = self->children.items[i];
            if (child->height.type == SIZE_TYPE_GROW)
            {
                child->rect.height = remaining_height;
            }
        }
    }
    else if (self->direction == DIRECTION_TOP_TO_BOTTOM)
    {
        if (self->children.len > 0)
        {
            remaining_height -= self->child_gap * (self->children.len - 1);
        }

        for (usize i = 0; i < self->children.len; i++)
        {
            Node *child = self->children.items[i];
            remaining_height -= child->rect.height;
            if (child->height.type == SIZE_TYPE_GROW)
            {
                node_ptr_array_add(&ui.arena, &growable, child);
            }
        }
        while (remaining_height > 0 && growable.len > 0)
        {
            f32 smallest = growable.items[0]->rect.height;
            f32 second_smallest = INFINITY;
            f32 height_to_add = remaining_height;
            for (usize i = 0; i < growable.len; i++)
            {
                Node *child = growable.items[i];
                if (child->rect.height < smallest)
                {
                    second_smallest = smallest;
                    smallest = child->rect.height;
                }
                if (child->rect.height > smallest)
                {
                    second_smallest = MIN(second_smallest, child->rect.height);
                    height_to_add = second_smallest - smallest;
                }
            }
            height_to_add = MIN(height_to_add, remaining_height / growable.len);
            if (height_to_add == 0)
                break;
            for (usize i = 0; i < growable.len; i++)
            {
                Node *child = growable.items[i];
                if (child->rect.height == smallest)
                {
                    child->rect.height += height_to_add;
                    remaining_height -= height_to_add;
                }
            }
        }
    }

    arena_quick_load(&ui.arena, arena_save);

    for (usize i = 0; i < self->children.len; i++)
    {
        node_grow_height(self->children.items[i]);
    }
}

static void container_position(Node *node)
{
    Contrainer *self = (Contrainer *)node;

    f32 left_offset = node->rect.x + node->paddings.right;
    f32 top_offset = node->rect.y + node->paddings.top;

    f32 remaining_width = node->rect.width;
    f32 remaining_height = node->rect.width;

    for (usize i = 0; i < self->children.len; i++)
    {
        Node *child = self->children.items[i];
        remaining_width -= child->rect.width;
        remaining_height -= child->rect.height;
    }

    if (self->direction == DIRECTION_LEFT_TO_RIGHT)
    {
        if (node->alignment == ALIGNMENT_CENTER)
        {
            left_offset += remaining_width / 2;
        }
        for (usize i = 0; i < self->children.len; i++)
        {
            Node *child = self->children.items[i];
            Vector2 child_position = {0};
            child_position.x += left_offset;
            if (node->alignment == ALIGNMENT_CENTER)
            {
                child_position.y = (node->rect.height - child->rect.height) / 2;
            }
            else
            {
                child_position.y += top_offset;
            }
            child->rect.x = child_position.x;
            child->rect.y = child_position.y;
            left_offset += child->rect.width + self->child_gap;
        }
    }
    else if (self->direction == DIRECTION_TOP_TO_BOTTOM)
    {
        if (node->alignment == ALIGNMENT_CENTER)
        {
            top_offset += remaining_height / 2;
        }
        for (usize i = 0; i < self->children.len; i++)
        {
            Node *child = self->children.items[i];
            Vector2 child_position = {0};
            if (node->alignment == ALIGNMENT_CENTER)
            {
                child_position.x = (node->rect.width - child->rect.width) / 2;
            }
            else
            {
                child_position.x += left_offset;
            }
            child_position.y += top_offset;
            child->rect.x = child_position.x;
            child->rect.y = child_position.y;
            top_offset += child->rect.height + self->child_gap;
        }
    }

    for (usize i = 0; i < self->children.len; i++)
    {
        node_position(self->children.items[i]);
    }
}

static void container_draw(Node *node)
{
    Contrainer *self = (Contrainer *)node;
    DrawRectangleRec(node->rect, node->background);
    for (usize i = 0; i < self->children.len; i++)
    {
        node_draw(self->children.items[i]);
    }
}

void container_open(ContainerOptions options)
{
    Contrainer *self = ARENA_PUSH_STRUCT_ZERO(&ui.arena, Contrainer);
    self->child_gap = options.child_gap;
    self->direction = options.direction;
    self->node.id = options.id;
    self->node.background = options.background;
    self->node.width = options.width;
    self->node.height = options.height;
    self->node.paddings = options.paddings;
    self->node.add_child = container_add_child;
    self->node.get_children = container_get_children;
    self->node.fit_width = container_fit_width;
    self->node.fit_height = container_fit_height;
    self->node.grow_width = container_grow_width;
    self->node.grow_height = container_grow_height;
    self->node.position = container_position;
    self->node.draw = container_draw;
    if (ui.current == NULL)
    {
        ui.current = &self->node;
    }
    else
    {
        node_add_child(ui.current, &self->node);
    }
    ui.current = &self->node;
}

void container_close(void)
{
    if (ui.current->parent != NULL)
    {
        ui.current = ui.current->parent;
    }
    else
    {
        ui_resize();
    }
}

static u32 text_fit_width(Node *node)
{
    Text *self = (Text *)node;
    return MeasureText(self->text, self->size);
}

static void text_draw(Node *node) {
    Text *self = (Text *)node;
    DrawText(self->text, node->rect.x, node->rect.y, self->size, BLACK);
}

void text_open(TextOptions options)
{
    Text *self = ARENA_PUSH_STRUCT_ZERO(&ui.arena, Text);

    self->node.id = options.id;
    self->text = options.text;
    self->wrap = options.wrap;
    self->size = options.size;

    // self->node.add_child = text_add_child;
    // self->node.get_children = text_get_children;
    self->node.fit_width = text_fit_width;
    // self->node.fit_height = text_fit_height;
    // self->node.grow_width = text_grow_width;
    // self->node.grow_height = text_grow_height;
    // self->node.position = text_position;
    self->node.draw = text_draw;
    if (ui.current == NULL)
    {
        ui.current = &self->node;
    }
    else
    {
        node_add_child(ui.current, &self->node);
    }
}

void ui_init(void)
{
    ui.arena = arena_create(MB(1));
}

void ui_deinit(void)
{
    arena_destroy(&ui.arena);
}

void ui_draw(void)
{
    ui.current->draw(ui.current);
}

void ui_resize(void)
{
    node_fit_width(ui.current);
    node_grow_width(ui.current);
    node_fit_height(ui.current);
    node_grow_height(ui.current);
    node_position(ui.current);
}

void ui_print_tree(void)
{
    if (ui.current)
    {
        node_print(ui.current, 0);
    }
}
