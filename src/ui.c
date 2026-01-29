#include "ui.h"

#include "SKN/arena.h"
#include "SKN/list.h"
#include "SKN/math.h"
#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <string.h>

// Size
Size size_fit(void)
{
    return (Size){SIZE_TYPE_FIT, 0};
}

Size size_grow(void)
{
    return (Size){SIZE_TYPE_GROW, 0};
}

Size size_fixed(float value)
{
    return (Size){SIZE_TYPE_FIXED, value};
}

// Context
Context context_create(Arena *arena)
{
    return (Context){
        .arena = arena,
    };
}

void context_draw(Context *ctx)
{
    node_draw(ctx->current);
}

void context_calc_layout(Context *ctx)
{
    node_calc_layout(ctx, ctx->current);
}

// Padding
Padding padding_all(int value)
{
    return (Padding){value, value, value, value};
}

// Node

void node_open(Context *ctx, Node options)
{
    Node *node = ARENA_PUSH_STRUCT(ctx->arena, Node);
    *node = options;
    if (ctx->current != NULL)
    {
        node->parent = ctx->current;
        LIST_APPEND(NodePtrList, ctx->arena, &ctx->current->children, node);
    }
    ctx->current = node;
}

void node_close(Context *context)
{
    if (context->current->parent != NULL)
    {
        context->current = context->current->parent;
    }
}

void node_print(Node *self, int level)
{
    for (int i = 0; i < level; i++)
    {
        printf("\t");
    }
    const char *id = self->id != NULL ? self->id : "NULL";
    printf("id: %s\n", id);

    LIST_FOREACH(NodePtrList, element, self->children)
    {
        node_print(element->value, level + 1);
    }
}

static void node_calc_fit_width(Node *self)
{
    LIST_FOREACH(NodePtrList, element, self->children)
    {
        Node *child = element->value;
        node_calc_fit_width(child);
    }

    float width = self->padding.left + self->padding.right;

    // child gap
    if (self->children.len > 0)
    {
        width += (self->children.len - 1) * self->child_gap;
    }

    LIST_FOREACH(NodePtrList, element, self->children)
    {
        Node *child = element->value;

        width += child->rect.width;
    }

    if (self->width.type == SIZE_TYPE_FIXED)
    {
        self->rect.width = self->width.value;
    }
    else
    {
        self->rect.width = width;
    }
}

static void node_calc_fit_height(Node *self)
{
    LIST_FOREACH(NodePtrList, element, self->children)
    {
        Node *child = element->value;
        node_calc_fit_height(child);
    }

    float height = self->padding.top + self->padding.bottom;
    float max_child_height = 0;

    LIST_FOREACH(NodePtrList, element, self->children)
    {
        Node *child = element->value;

        max_child_height = max(child->rect.height, max_child_height);
    }

    height += max_child_height;

    if (self->height.type == SIZE_TYPE_FIXED)
    {
        self->rect.height = self->height.value;
    }
    else
    {
        self->rect.height = height;
    }
}

static void node_calc_grow_width(Context *ctx, Node *self)
{
    float remaining_width = self->rect.width;
    remaining_width -= self->padding.left + self->padding.right;
    size_t arena_save = arena_quick_save(ctx->arena);
    NodePtrList growable = {0};
    if (self->children.len > 0)
    {
        remaining_width -= self->child_gap * (self->children.len - 1);
    }

    LIST_FOREACH(NodePtrList, element, self->children)
    {
        Node *child = element->value;
        remaining_width -= child->rect.width;
        if (child->width.type == SIZE_TYPE_GROW)
        {
            LIST_APPEND(NodePtrList, ctx->arena, &growable, child);
        }
    }
    while (remaining_width > 0 && growable.len > 0)
    {
        float smallest = growable.item->value->rect.width;
        float second_smallest = INFINITY;
        float width_to_add = remaining_width;
        LIST_FOREACH(NodePtrList, element, growable)
        {
            Node *child = element->value;
            if (child->rect.width < smallest)
            {
                second_smallest = smallest;
                smallest = child->rect.width;
            }
            if (child->rect.width > smallest)
            {
                second_smallest = min(second_smallest, child->rect.width);
                width_to_add = second_smallest - smallest;
            }
        }
        width_to_add = min(width_to_add, remaining_width / growable.len);
        if (width_to_add == 0)
            break;
        LIST_FOREACH(NodePtrList, element, growable)
        {
            Node *child = element->value;
            if (child->rect.width == smallest)
            {
                child->rect.width += width_to_add;
                remaining_width -= width_to_add;
            }
        }
    }

    arena_quick_load(ctx->arena, arena_save);

    LIST_FOREACH(NodePtrList, element, self->children)
    {
        Node *child = element->value;
        node_calc_grow_width(ctx, child);
    }
}

static void node_calc_grow_height(Context *ctx, Node *self)
{
    float remaining_height = self->rect.height;
    remaining_height -= self->padding.top + self->padding.bottom;

    LIST_FOREACH(NodePtrList, element, self->children)
    {
        Node *child = element->value;
        if (child->height.type == SIZE_TYPE_GROW)
        {
            child->rect.height = remaining_height;
        }
    }

    LIST_FOREACH(NodePtrList, element, self->children)
    {
        Node *child = element->value;
        node_calc_grow_height(ctx, child);
    }
}

static void node_calc_position(Node *self)
{

    float x_offset = self->rect.x + self->padding.right;
    float y_offset = self->rect.y + self->padding.top;

    LIST_FOREACH(NodePtrList, element, self->children)
    {
        Node *child = element->value;

        child->rect.x = x_offset;
        child->rect.y = y_offset;

        x_offset += child->rect.width + self->child_gap;
    }

    LIST_FOREACH(NodePtrList, element, self->children)
    {
        Node *child = element->value;
        node_calc_position(child);
    }
}

void node_calc_layout(Context *ctx, Node *node)
{
    (void)ctx;
    node_calc_fit_width(node);
    node_calc_grow_width(ctx, node);
    node_calc_fit_height(node);
    node_calc_grow_height(ctx, node);
    node_calc_position(node);
}

void node_draw(Node *self)
{
    DrawRectangleRec(self->rect, self->color);
    LIST_FOREACH(NodePtrList, element, self->children)
    {
        node_draw(element->value);
    }
}
