#include "ui.h"
#include "SKN/arena.h"
#include "SKN/array.h"
#include "SKN/types.h"

#include <assert.h>
#include <math.h>
#include <raylib.h>
#include <stdio.h>

#include <SKN/math.h>
#include <raymath.h>
#include <string.h>

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

DYNAMIC_ARRAY_IMPL_ADD(NodePtrArray, Node *, node_ptr_array_add)
DYNAMIC_ARRAY_IMPL_SWAP_REMOVE(NodePtrArray, node_ptr_array_swap_remove)

static void container_add_child(Contrainer *self, Node *child)
{
    child->parent = self;
    node_ptr_array_add(&ui.arena, &self->children, child);
}

static u32 container_fit_width(Node *node)
{
    Contrainer *self = (Contrainer *)node;

    for (usize i = 0; i < self->children.len; i++)
    {
        Node *child = self->children.items[i];
        assert(child->fit_width);
        child->rect.width = child->fit_width(child);
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
    else // DIRECTION_TOP_TO_BOTTOM
    {
        f32 max_width = 0;
        for (usize i = 0; i < self->children.len; i++)
        {
            max_width = MAX(self->children.items[i]->rect.width, max_width);
        }
        return max_width + node->paddings.left + node->paddings.right;
    }
}

static u32 container_fit_height(Node *node)
{
    Contrainer *self = (Contrainer *)node;

    for (usize i = 0; i < self->children.len; i++)
    {
        Node *child = self->children.items[i];
        assert(child->fit_height);
        child->rect.height = child->fit_height(child);
    }

    if (node->height.type == SIZE_TYPE_FIXED)
    {
        return node->height.value;
    }

    if (self->direction == DIRECTION_TOP_TO_BOTTOM)
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
        return total_height;
    }
    else // DIRECTION_LEFT_TO_RIGHT
    {
        f32 max_height = 0;
        for (usize i = 0; i < self->children.len; i++)
        {
            max_height = MAX(self->children.items[i]->rect.height, max_height);
        }
        return max_height + node->paddings.top + node->paddings.bottom;
    }
}

static void container_grow_width(Node *node)
{
    Contrainer *self = (Contrainer *)node;

    f32 remaining_width = node->rect.width;
    remaining_width -= node->paddings.left + node->paddings.right;
    ArenaSave save = arena_quick_save(&ui.arena);
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

    NodePtrArray text_nodes = {0};

    if (self->direction == DIRECTION_LEFT_TO_RIGHT)
    {
        for (usize i = 0; i < self->children.len; i++)
        {
            Node *child = self->children.items[i];
            if (child->is_text)
            {
                Text *child_text = (Text *)child;
                if (child_text->wrap == true)
                {
                    node_ptr_array_add(&ui.arena, &text_nodes, child);
                }
            }
        }

        while (remaining_width > 0 && text_nodes.len > 0)
        {
            f32 smallest = text_nodes.items[0]->rect.width;
            f32 second_smallest = INFINITY;
            f32 width_to_add = remaining_width;

            for (usize i = 0; i < text_nodes.len; i++)
            {
                Node *child = text_nodes.items[i];
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

            width_to_add = MIN(width_to_add, remaining_width / text_nodes.len);
            if (width_to_add == 0)
                break;

            for (usize i = 0; i < text_nodes.len; i++)
            {
                Node *child = text_nodes.items[i];
                if (child->rect.width == smallest)
                {
                    Text *text_child = (Text *)child;
                    u32 max_text_width = MeasureText(text_child->text, text_child->size);
                    if (child->rect.width + width_to_add > max_text_width)
                    {
                        width_to_add = max_text_width - child->rect.width;
                        child->rect.width += width_to_add;
                        remaining_width -= width_to_add;
                        node_ptr_array_swap_remove(&text_nodes, i);
                        break;
                    }
                    else
                    {
                        child->rect.width += width_to_add;
                        remaining_width -= width_to_add;
                    }
                }
            }
        }
    }
    else
    {
        for (usize i = 0; i < self->children.len; i++)
        {
            Node *child = self->children.items[i];
            if (child->is_text)
            {
                child->rect.width = remaining_width;
            }
        }
    }

    arena_quick_load(&ui.arena, save);

    for (usize i = 0; i < self->children.len; i++)
    {
        Node *child = self->children.items[i];
        if (child->grow_width != NULL)
        {
            child->grow_width(child);
        }
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
        Node *child = self->children.items[i];
        if (child->grow_height != NULL)
        {
            child->grow_height(child);
        }
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
        Node *child = self->children.items[i];
        if (child->position != NULL)
        {
            child->position(child);
        }
    }
}

static void container_draw(Node *node)
{
    Contrainer *self = (Contrainer *)node;
    DrawRectangleRec(node->rect, node->background);
    for (usize i = 0; i < self->children.len; i++)
    {
        Node *child = self->children.items[i];
        assert(child->draw);
        child->draw(child);
    }
}

static void node_print_base(Node *node, usize level, const char *type)
{
    for (usize i = 0; i < level; i++)
    {
        printf("\t");
    }
    printf("%s(id = %s)\n", type, node->id);
}

static void container_print(Node *node, usize level)
{
    Contrainer *self = (Contrainer *)node;
    node_print_base(node, level, "Contrainer");
    for (usize i = 0; i < self->children.len; i++)
    {
        Node *child = self->children.items[i];
        if (child->print != NULL)
        {
            child->print(child, level + 1);
        }
        else
        {
            node_print_base(child, level + 1, "Unknown");
        }
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
    self->node.fit_width = container_fit_width;
    self->node.fit_height = container_fit_height;
    self->node.grow_width = container_grow_width;
    self->node.grow_height = container_grow_height;
    self->node.position = container_position;
    self->node.draw = container_draw;
    self->node.print = container_print;

    if (ui.current != NULL)
    {
        container_add_child(ui.current, &self->node);
    }

    ui.current = self;
}

void container_close(void)
{
    if (ui.current->node.parent != NULL)
    {
        ui.current = ui.current->node.parent;
    }
    else
    {
        ui_resize(0, 0);
    }
}

static u32 text_fit_width(Node *node)
{
    Text *self = (Text *)node;
    if (self->wrap)
    {
        f32 min_width = 0;

        ArenaSave save = arena_quick_save(&ui.arena);
        char *text = arena_strdup(&ui.arena, self->text);
        const char *token = strtok(text, " ");
        while (token != NULL)
        {
            u32 width = MeasureText(token, 20);
            min_width = MAX(width, min_width);
            token = strtok(NULL, " ");
        }
        node->rect.width = min_width;
        arena_quick_load(&ui.arena, save);

        return min_width;
    }
    else
    {
        return MeasureText(self->text, self->size);
    }
}

static u32 text_fit_height(Node *node)
{
    Text *self = (Text *)node;
    return self->size;
}

DYNAMIC_ARRAY_IMPL_ADD(StringArray, char *, string_array_add)

static char *concat(char **words, usize words_len)
{
    assert(words_len > 0);
    usize total_len = (words_len - 1);
    for (usize i = 0; i < words_len; i++)
    {
        total_len += strlen(words[i]);
    }
    total_len += 1; // for \0
    char *line = arena_push_zero(&ui.arena, sizeof(char) * total_len);
    usize offset = 0;
    for (usize i = 0; i < words_len; i++)
    {
        const char *word = words[i];
        strcpy(line + offset, word);
        offset += strlen(word);
        if (i + 1 != words_len)
        {
            line[offset] = ' ';
        }
        offset += 1;
    }
    return line;
}

// Make sure to "quick save" before and "quick load" after the call
static StringArray wrap_text(const char *original_text, u32 width, u32 height)
{
    StringArray lines = {0};
    StringArray words = {0};
    char *text = arena_strdup(&ui.arena, original_text);
    char *token = strtok(text, " ");
    while (token != NULL)
    {
        string_array_add(&ui.arena, &words, token);
        token = strtok(NULL, " ");
    }
    usize offset = 0;
    while (offset < words.len)
    {
        char *line = words.items[offset];
        usize len = 1;
        for (usize i = 2; i < (words.len - offset + 1); i++)
        {
            char *new_line = concat(words.items + offset, i);
            if ((u32)MeasureText(new_line, height) <= width)
            {
                len = i;
                line = new_line;
            }
        }
        string_array_add(&ui.arena, &lines, line);
        offset += len;
    }
    return lines;
}

static void text_draw(Node *node)
{
    Text *self = (Text *)node;
    if (self->wrap)
    {
        ArenaSave save = arena_quick_save(&ui.arena);
        StringArray lines = wrap_text(self->text, node->rect.width, node->rect.height);
        u32 left_offest = node->rect.x;
        u32 top_offest = node->rect.y;
        for (usize i = 0; i < lines.len; i++)
        {
            DrawText(lines.items[i], left_offest, top_offest, self->size, BLACK);
            top_offest += self->size;
        }
        arena_quick_load(&ui.arena, save);
    }
    else
    {
        DrawText(self->text, node->rect.x, node->rect.y, self->size, BLACK);
    }
}

static void text_print(Node *node, usize level)
{
    node_print_base(node, level, "Text");
}

void text_open(TextOptions options)
{
    Text *self = ARENA_PUSH_STRUCT_ZERO(&ui.arena, Text);

    self->node.id = options.id;
    self->text = options.text;
    self->wrap = options.wrap;
    self->node.is_text = true;
    self->size = options.size;
    self->node.fit_width = text_fit_width;
    self->node.fit_height = text_fit_height;
    self->node.draw = text_draw;
    self->node.print = text_print;
    assert(ui.current != NULL);
    container_add_child(ui.current, &self->node);
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
    ui.current->node.draw(&ui.current->node);
}

void ui_resize(u32 width, u32 height)
{
    Node *current = &ui.current->node;

    if (width != 0)
    {
        current->width = size_fixed(width);
    }
    if (height != 0)
    {
        current->height = size_fixed(height);
    }

    assert(current->fit_width);
    current->rect.width = current->fit_width(current);

    if (current->grow_width != NULL)
    {
        current->grow_width(current);
    }

    assert(current->fit_height);
    current->rect.height = current->fit_height(current);

    if (current->grow_height != NULL)
    {
        current->grow_height(current);
    }

    if (current->position != NULL)
    {
        current->position(current);
    }
}

void ui_print_tree(void)
{
    assert(ui.current != NULL);
    container_print(&ui.current->node, 0);
}
