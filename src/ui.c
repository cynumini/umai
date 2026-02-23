#include "ui.h"
#include "SKN/arena.h"
#include "SKN/array.h"
#include "SKN/types.h"

#include <assert.h>
#include <math.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>

#include <SKN/math.h>
#include <raymath.h>
#include <string.h>

UI ui = {.update = true};

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

static u32 container_fit_width(Node *node)
{
    Container *self = (Container *)node;

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

    u32 self_width = node->paddings.left + node->paddings.right + (node->border_size * 2);

    if (self->direction == DIRECTION_LEFT_TO_RIGHT)
    {
        f32 total_width = self_width;
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
        return max_width + self_width;
    }
}

static u32 container_fit_height(Node *node)
{
    Container *self = (Container *)node;

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

    u32 self_height = node->paddings.top + node->paddings.bottom + (node->border_size * 2);

    if (self->direction == DIRECTION_TOP_TO_BOTTOM)
    {
        f32 total_height = self_height;
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
        return max_height + self_height;
    }
}

DYNAMIC_ARRAY_IMPL_ADD(TextPtrArray, Text *, text_ptr_array_add)
DYNAMIC_ARRAY_IMPL_SWAP_REMOVE(TextPtrArray, text_ptr_array_swap_remove)

static void container_grow_width(Container *self)
{
    Node *node = (Node *)self;

    f32 remaining_width = node->rect.width;
    remaining_width -= node->paddings.left + node->paddings.right + (node->border_size * 2);
    ArenaSave save = arena_quick_save(self->node.arena);
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
                node_ptr_array_add(self->node.arena, &growable, child);
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
                child->rect.width = MAX(remaining_width, child->rect.width);
            }
        }
    }

    TextPtrArray text_nodes = {0};

    if (self->direction == DIRECTION_LEFT_TO_RIGHT)
    {
        for (usize i = 0; i < self->children.len; i++)
        {
            Node *child = self->children.items[i];
            if (child->type == NODE_TYPE_TEXT)
            {
                Text *child_text = (Text *)child;
                if (child_text->wrap == true)
                {
                    text_ptr_array_add(self->node.arena, &text_nodes, child_text);
                }
            }
        }

        while (remaining_width > 0 && text_nodes.len > 0)
        {
            f32 smallest = text_nodes.items[0]->node.rect.width;
            f32 second_smallest = INFINITY;
            f32 width_to_add = remaining_width;

            for (usize i = 0; i < text_nodes.len; i++)
            {
                Text *child = text_nodes.items[i];
                if (child->node.rect.width < smallest)
                {
                    second_smallest = smallest;
                    smallest = child->node.rect.width;
                }
                if (child->node.rect.width > smallest)
                {
                    second_smallest = MIN(second_smallest, child->node.rect.width);
                    width_to_add = second_smallest - smallest;
                }
            }

            width_to_add = MIN(width_to_add, remaining_width / text_nodes.len);
            if (width_to_add == 0)
                break;

            for (usize i = 0; i < text_nodes.len; i++)
            {
                Text *child = text_nodes.items[i];
                if (child->node.rect.width == smallest)
                {
                    u32 max_text_width = MeasureText(child->text, child->size);
                    if (child->node.rect.width + width_to_add > max_text_width)
                    {
                        width_to_add = max_text_width - child->node.rect.width;
                        child->node.rect.width += width_to_add;
                        remaining_width -= width_to_add;
                        text_ptr_array_swap_remove(&text_nodes, i);
                        break;
                    }
                    else
                    {
                        child->node.rect.width += width_to_add;
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
            if (child->type == NODE_TYPE_TEXT)
            {
                child->rect.width = remaining_width;
            }
        }
    }

    arena_quick_load(self->node.arena, save);

    for (usize i = 0; i < self->children.len; i++)
    {
        Node *child = self->children.items[i];
        if (child->type == NODE_TYPE_CONTRAINER)
        {
            Container *container = (Container *)child;
            assert(container->grow_width);
            container->grow_width(container);
        }
    }
}

static void container_grow_height(Container *self)
{
    Node *node = (Node *)self;
    f32 remaining_height = node->rect.height;
    remaining_height -= node->paddings.top + node->paddings.bottom + (node->border_size * 2);
    ArenaSave arena_save = arena_quick_save(self->node.arena);
    NodePtrArray growable = {0};

    if (self->direction == DIRECTION_LEFT_TO_RIGHT)
    {
        for (usize i = 0; i < self->children.len; i++)
        {
            Node *child = self->children.items[i];
            if (child->height.type == SIZE_TYPE_GROW)
            {
                child->rect.height = MAX(remaining_height, child->rect.height);
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
                node_ptr_array_add(self->node.arena, &growable, child);
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

    arena_quick_load(self->node.arena, arena_save);

    for (usize i = 0; i < self->children.len; i++)
    {
        Node *child = self->children.items[i];
        if (child->type == NODE_TYPE_CONTRAINER)
        {
            Container *container = (Container *)child;
            assert(container->grow_height);
            container->grow_height(container);
        }
    }
}

static void container_position(Container *self)
{
    Node *self_node = (Node *)self;

    f32 left_offset = self_node->rect.x + self_node->paddings.right + self_node->border_size;
    f32 top_offset = self_node->rect.y + self_node->paddings.top + self_node->border_size;

    f32 remaining_width = self_node->rect.width;
    f32 remaining_height = self_node->rect.width;

    for (usize i = 0; i < self->children.len; i++)
    {
        Node *child = self->children.items[i];
        remaining_width -= child->rect.width;
        remaining_height -= child->rect.height;
    }

    if (self->direction == DIRECTION_LEFT_TO_RIGHT)
    {
        if (self_node->alignment == ALIGNMENT_CENTER)
        {
            left_offset += remaining_width / 2;
        }
        for (usize i = 0; i < self->children.len; i++)
        {
            Node *child = self->children.items[i];
            Vector2 child_position = {0};
            child_position.x += left_offset;
            child_position.y = top_offset;
            if (self_node->alignment == ALIGNMENT_CENTER)
            {
                child_position.y += (self_node->rect.height - child->rect.height) / 2;
            }
            child->rect.x = child_position.x;
            child->rect.y = child_position.y;
            left_offset += child->rect.width + self->child_gap;
        }
    }
    else if (self->direction == DIRECTION_TOP_TO_BOTTOM)
    {
        if (self_node->alignment == ALIGNMENT_CENTER)
        {
            top_offset += remaining_height / 2;
        }
        for (usize i = 0; i < self->children.len; i++)
        {
            Node *child = self->children.items[i];
            Vector2 child_position = {0};
            if (self_node->alignment == ALIGNMENT_CENTER)
            {
                child_position.x = (self_node->rect.width - child->rect.width) / 2;
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
        if (child->type == NODE_TYPE_CONTRAINER)
        {
            Container *child_container = (Container *)child;
            child_container->position(child_container);
        }
    }
}

static void container_draw(Node *node)
{
    Container *self = (Container *)node;
    DrawRectangleRec(node->rect, node->background);
    for (usize i = 0; i < self->children.len; i++)
    {
        Node *child = self->children.items[i];
        assert(child->draw);
        child->draw(child);
    }
    if (self->node.border_size > 0)
    {
        DrawRectangleLinesEx(node->rect, self->node.border_size, node->border_color);
    }
}

static void container_update(Node *self_node, Event event)
{
    Container *self = (Container *)self_node;
    for (usize i = 0; i < self->children.len; i++)
    {
        Node *child = self->children.items[i];
        Event child_event = event;
        if (event.mouse_hover)
        {
            if (!CheckCollisionPointRec(ui.mouse_position, child->rect))
            {
                child_event.mouse_hover = false;
            }
        }
        assert(child->update);
        child->update(child, child_event);
    }
}

static void node_print_base(Node *node, usize level, const char *name)
{
    for (usize i = 0; i < level; i++)
    {
        printf("\t");
    }
    printf("%s(id = %s)\n", name, node->id);
}

static void container_base_print(Node *node, usize level, const char *name)
{
    Container *self = (Container *)node;
    node_print_base(node, level, name);
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

static void container_print(Node *node, usize level)
{
    container_base_print(node, level, "Container");
}

static void node_setup(Node *node, Arena *arena, const char *id, NodeType type)
{
    node->id = id;
    node->type = type;
    node->arena = arena;
}

static void container_setup(Container *self, Arena *arena, ContainerOptions options)
{
    node_setup(&self->node, arena, options.id, NODE_TYPE_CONTRAINER);
    self->node.background = options.background;
    self->node.width = options.width;
    self->node.height = options.height;
    self->node.paddings = options.paddings;
    self->node.alignment = options.alignment;
    self->node.border_color = options.border_color;
    self->node.border_size = options.border_size;
    self->child_gap = options.child_gap;
    self->direction = options.direction;

    self->node.fit_width = container_fit_width;
    self->node.fit_height = container_fit_height;
    self->node.draw = container_draw;
    self->node.update = container_update;
    self->node.print = container_print;

    self->grow_width = container_grow_width;
    self->grow_height = container_grow_height;
    self->position = container_position;
}

void container_open(ContainerOptions options)
{
    Container *self = ARENA_PUSH_STRUCT_ZERO(&ui.arena, Container);
    container_setup(self, &ui.arena, options);
    if (ui.root != NULL)
    {
        container_add_child(ui.root, &self->node);
    }
    ui.root = self;
}

void container_close(void)
{
    if (ui.root->node.parent != NULL)
    {
        ui.root = ui.root->node.parent;
    }
    else
    {
        ui_resize(0, 0);
    }
}

Container *container_create(Arena *arena, ContainerOptions options)
{
    Container *self = ARENA_PUSH_STRUCT_ZERO(arena, Container);
    container_setup(self, arena, options);
    return self;
}

void container_add_child(Container *self, Node *child)
{
    child->parent = self;
    node_ptr_array_add(self->node.arena, &self->children, child);
}

static u32 text_fit_width(Node *node)
{
    Text *self = (Text *)node;
    if (self->wrap)
    {
        f32 min_width = 0;

        ArenaSave save = arena_quick_save(self->node.arena);
        char *text = arena_strdup(self->node.arena, self->text);
        const char *token = strtok(text, " ");
        while (token != NULL)
        {
            u32 width = MeasureText(token, 20);
            min_width = MAX(width, min_width);
            token = strtok(NULL, " ");
        }
        node->rect.width = min_width;
        arena_quick_load(self->node.arena, save);

        return min_width;
    }
    else
    {
        return MeasureText(self->text, self->size);
    }
}

static u32 measure_text(const char *text, usize len, int font_size)
{
    assert(len > 0);
    ArenaSave save = arena_quick_save(&ui.arena);
    const char *temp_text = arena_strndup(&ui.arena, text, len);
    usize result = MeasureText(temp_text, font_size);
    arena_quick_load(&ui.arena, save);
    return result;
}

// Make sure to "quick save" before and "quick load" after the call
static const char *wrap_text(const char *text, u32 width, u32 height)
{
    char *new_text = arena_strdup(&ui.arena, text);
    usize end = 0;
    for (usize i = 0, start = 0; i < strlen(new_text);)
    {
        if (new_text[i] == ' ')
        {
            if (measure_text(new_text + start, i - start, height) <= width)
            {
                end = i;
            }
            else
            {
                new_text[end] = '\n';
                end += 1;
                start = end;
                i = end;
                continue;
            }
        }
        i++;
    }
    if ((u32)MeasureText(new_text, height) > width)
    {
        new_text[end] = '\n';
    }
    return new_text;
}

static u32 text_fit_height(Node *node)
{
    Text *self = (Text *)node;
    ArenaSave save = arena_quick_save(&ui.arena);
    const char *text = self->text;
    if (self->wrap)
    {
        text = wrap_text(text, node->rect.width, self->size);
    }
    u32 height = MeasureTextEx(GetFontDefault(), text, self->size, 0).y;
    arena_quick_load(&ui.arena, save);
    return height;
}

static void text_update(Node *self_node, Event event)
{
    (void)self_node;
    (void)event;
}

static void text_draw(Node *node)
{
    Text *self = (Text *)node;
    ArenaSave save = arena_quick_save(&ui.arena);
    const char *text = self->text;
    if (self->wrap)
    {
        text = wrap_text(text, node->rect.width, self->size);
    }
    arena_quick_load(&ui.arena, save);
    DrawText(text, node->rect.x, node->rect.y, self->size, BLACK);
}

static void text_print(Node *node, usize level)
{
    node_print_base(node, level, "Text");
}

static void text_setup(Text *self, const char *text, TextOptions options)
{
    self->node.id = options.id;
    self->node.type = NODE_TYPE_TEXT;
    self->node.print = text_print;

    self->node.fit_width = text_fit_width;
    self->node.fit_height = text_fit_height;
    self->node.update = text_update;
    self->node.draw = text_draw;

    self->text = text;
    self->wrap = options.wrap;
    self->size = options.size == 0 ? 20 : options.size;
}

void text_open(const char *text, TextOptions options)
{
    Text *self = ARENA_PUSH_STRUCT_ZERO(&ui.arena, Text);
    text_setup(self, text, options);
    assert(ui.root != NULL);
    container_add_child(ui.root, &self->node);
}

Text *text_create(Arena *arena, const char *text, TextOptions options)
{
    Text *self = ARENA_PUSH_STRUCT_ZERO(arena, Text);
    text_setup(self, text, options);
    return self;
}

// Button
static void button_update(Node *self_node, Event event)
{
    Button *self = (Button *)self_node;
    if (event.mouse_hover)
    {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            self_node->background = DARKGRAY;
        }
        else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            if (self->on_click)
            {
                self->on_click(self);
            }
        }
        else
        {
            self_node->background = BLUE;
        }
    }
    else
    {
        self_node->background = GRAY;
    }
}

static void button_print(Node *node, usize level)
{
    container_base_print(node, level, "Button");
}

static void button_setup(Button *self, Arena *arena, const char *text, ButtonOptions options)
{
    container_setup(&self->container, arena,
                    (ContainerOptions){
                        .id = options.id,
                        .paddings = sides_all(8),
                        .background = GRAY,
                        .border_size = 1,
                        .border_color = BLACK,
                    });
    self->container.node.update = button_update;
    self->container.node.print = button_print;
    text_setup(&self->text, text, (TextOptions){0});

    container_add_child(&self->container, (Node *)&self->text);

    self->user_data = options.user_data;
    self->on_click = options.on_click;
}

Button *button_create(Arena *arena, const char *text, ButtonOptions options)
{
    Button *self = ARENA_PUSH_STRUCT_ZERO(arena, Button);

    button_setup(self, arena, text, options);

    return self;
}
// static u32 tabs_fit_width(Node *node)
// {
//     Container *self = (Container *)node;
//
//     f32 max_width = 0;
//
//     for (usize i = 0; i < self->children.len; i++)
//     {
//         Node *child = self->children.items[i];
//         assert(child->fit_width);
//         child->rect.width = child->fit_width(child);
//         max_width = MAX(child->rect.width, max_width);
//     }
//
//     if (node->width.type == SIZE_TYPE_FIXED)
//     {
//         return node->width.value;
//     }
//     else
//     {
//         return max_width;
//     }
// }
//
// static u32 tabs_fit_height(Node *node)
// {
//     Container *self = (Container *)node;
//
//     f32 max_height = 0;
//
//     for (usize i = 0; i < self->children.len; i++)
//     {
//         Node *child = self->children.items[i];
//         assert(child->fit_height);
//         child->rect.height = child->fit_height(child);
//         max_height = MAX(child->rect.height, max_height);
//     }
//
//     if (node->height.type == SIZE_TYPE_FIXED)
//     {
//         return node->height.value;
//     }
//     else
//     {
//         return max_height;
//     }
// }
//
// static void tabs_grow_width(Container *self_container)
// {
//     Tabs *self = (Tabs *)self_container;
//     Node *self_node = (Node *)self;
//
//     for (usize i = 0; i < self_container->children.len; i++)
//     {
//         Node *child = self_container->children.items[i];
//         if (child->width.type == SIZE_TYPE_GROW)
//         {
//             child->rect.width = self_node->rect.width;
//         }
//         if (child->type == NODE_TYPE_TEXT)
//         {
//             Text *child_text = (Text *)child;
//             u32 max_text_width = MeasureText(child_text->text, child_text->size);
//             child->rect.width = MIN(self_node->rect.width, max_text_width);
//         }
//         if (child->type == NODE_TYPE_CONTRAINER)
//         {
//             Container *child_container = (Container *)child;
//             assert(child_container->grow_width);
//             child_container->grow_width(child_container);
//         }
//     }
// }
//
// static void tabs_grow_height(Container *self_container)
// {
//     Tabs *self = (Tabs *)self_container;
//     Node *self_node = (Node *)self;
//
//     for (usize i = 0; i < self_container->children.len; i++)
//     {
//         Node *child = self_container->children.items[i];
//         if (child->height.type == SIZE_TYPE_GROW)
//         {
//             child->rect.height = self_node->rect.height - 20; // TODO: Font size for tabs name
//         }
//         if (child->type == NODE_TYPE_CONTRAINER)
//         {
//             Container *child_container = (Container *)child;
//             assert(child_container->grow_height);
//             child_container->grow_height(child_container);
//         }
//     }
// }
//
// static void tabs_positions(Container *self_container)
// {
//
//     Tabs *self = (Tabs *)self_container;
//     Node *self_node = (Node *)self;
//     for (usize i = 0; i < self_container->children.len; i++)
//     {
//         Node *child = self_container->children.items[i];
//         child->rect.x = self_node->rect.x;
//         child->rect.y = self_node->rect.y + 20; // TODO: Font size for tabs name
//         if (child->type == NODE_TYPE_CONTRAINER)
//         {
//             Container *child_container = (Container *)child;
//             assert(child_container->position);
//             child_container->position(child_container);
//         }
//     }
// }
//
// static void tabs_print(Node *node, usize level)
// {
//     container_base_print(node, level, "Tabs");
// }
//
// static void tabs_draw(Node *self_node)
// {
//     Tabs *self = (Tabs *)self_node;
//     Container *self_container = (Container *)self;
//
//     f32 x_offset = self_node->rect.x;
//     f32 y_offset = self_node->rect.y;
//     DrawRectangleRec((Rectangle){x_offset, y_offset, self_node->rect.width, 20},
//                      GRAY); // TODO: Add font size for tabs & color for inactive tab
//     for (usize i = 0; i < self_container->children.len; i++)
//     {
//         Node *child = self_container->children.items[i];
//         const char *text = child->id;
//         f32 width = MeasureText(text, 20);
//         if (i == self->current)
//         {
//             DrawRectangleRec((Rectangle){x_offset, y_offset, width, 20}, WHITE); // TODO: Use background color
//         }
//         DrawText(text, x_offset, y_offset, 20, BLACK);
//         x_offset += width; // TODO: tab gap
//     }
//
//     Node *child = self_container->children.items[self->current];
//     assert(child->draw);
//     child->draw(child);
// }
//
// void tabs_open(TabsOptions options)
// {
//     Tabs *self = ARENA_PUSH_STRUCT_ZERO(&ui.arena, Tabs);
//     self->container.grow_width = tabs_grow_width;
//     self->container.grow_height = tabs_grow_height;
//     self->container.position = tabs_positions;
//     self->container.node.id = options.id;
//     self->container.node.width = options.width;
//     self->container.node.height = options.width;
//     self->container.node.type = NODE_TYPE_CONTRAINER;
//     self->container.node.fit_width = tabs_fit_width;
//     self->container.node.fit_height = tabs_fit_height;
//     self->container.node.print = tabs_print;
//     self->container.node.draw = tabs_draw;
//
//     if (ui.root != NULL)
//     {
//         container_add_child(ui.root, &self->container.node);
//     }
//
//     ui.root = &self->container;
// }

// Tabs
Tabs *tabs_create(Arena *arena, TabsOptions options)
{
    Tabs *self = ARENA_PUSH_STRUCT_ZERO(arena, Tabs);

    container_setup(
        &self->root, arena,
        (ContainerOptions){
            .id = options.id, .width = options.width, .height = options.height, .direction = DIRECTION_TOP_TO_BOTTOM});
    container_setup(&self->top, arena,
                    (ContainerOptions){.width = size_grow(), .background = DARKGRAY, .child_gap = 8});
    container_setup(&self->bottom, arena, (ContainerOptions){0});
    container_add_child(&self->root, (Node *)&self->top);
    container_add_child(&self->root, (Node *)&self->bottom);

    return self;
}

static void tab_update(Node *self_node, Event event)
{
    Tab *self = (Tab *)self_node;
    container_update(self_node, event);
    if (self->active)
    {
        self->root.node.background = DARKGRAY;
    }
    else
    {
        self->root.node.background = GRAY;
    }
}

static Tab *tab_create(Arena *arena, const char *name, TabOptions options)
{
    Tab *self = ARENA_PUSH_STRUCT_ZERO(arena, Tab);

    self->closable = options.closable;

    container_setup(&self->root, arena, (ContainerOptions){.alignment = ALIGNMENT_CENTER, .child_gap = 4});
    self->root.node.update = tab_update;
    text_setup(&self->text, name, (TextOptions){0});
    button_setup(&self->button, arena, "x", (ButtonOptions){0});

    container_add_child(&self->root, (Node *)&self->text);
    container_add_child(&self->root, (Node *)&self->button);

    return self;
}

void tabs_select_tab(Tabs *self, usize index)
{
    for (usize i = 0; i < self->top.children.len; i++)
    {
        Node *child = self->top.children.items[i];
        Tab *child_tab = (Tab *)child;
        if (i == index)
        {
            child_tab->active = true;
        }
        else
        {
            child_tab->active = false;
        }
    }
}

void tabs_add_tab(Tabs *self, const char *name, TabOptions options, Node *node)
{
    Node *self_node = &self->root.node;
    Tab *tab = tab_create(self_node->arena, name, options);
    container_add_child(&self->top, (Node *)tab);
    container_add_child(&self->bottom, node);
    tabs_select_tab(self, self->top.children.len - 1);
}

void ui_init(void)
{
    ui.arena = arena_create(KB(5));
    // ui.arena = arena_create(KB(84));
}

void ui_deinit(void)
{
    arena_destroy(&ui.arena);
}

void ui_draw(void)
{
    assert(ui.root->node.draw);
    ui.root->node.draw(&ui.root->node);
}

void ui_resize(u32 width, u32 height)
{
    Node *node = &ui.root->node;

    if (width != 0)
    {
        node->width = size_fixed(width);
    }
    if (height != 0)
    {
        node->height = size_fixed(height);
    }

    assert(node->fit_width);
    assert(ui.root->grow_width);
    assert(node->fit_height);
    assert(ui.root->grow_height);
    assert(ui.root->position);

    node->rect.width = node->fit_width(node);
    ui.root->grow_width(ui.root);
    node->rect.height = node->fit_height(node);
    ui.root->grow_height(ui.root);
    ui.root->position(ui.root);
}

void ui_print_tree(void)
{
    assert(ui.root);
    assert(ui.root->node.print);
    ui.root->node.print(&ui.root->node, 0);
}

static Node *get_by_id_from_container(Container *container, const char *id)
{
    for (usize i = 0; container->children.len; i++)
    {
        Node *child = container->children.items[i];
        if (child->id != NULL && strcmp(child->id, id) == 0)
        {
            return child;
        }
        else if (child->type == NODE_TYPE_CONTRAINER)
        {
            Node *result = get_by_id_from_container((Container *)child, id);
            if (result != NULL)
            {
                return result;
            }
        }
    }
    return NULL;
}

Node *ui_get_by_id(const char *id)
{
    if (ui.root->node.id != NULL && strcmp(ui.root->node.id, id) == 0)
    {
        return (Node *)ui.root;
    }

    if (ui.root->node.type == NODE_TYPE_CONTRAINER)
    {
        return get_by_id_from_container((Container *)ui.root, id);
    }

    return NULL;
}

void ui_update(void)
{
    ui.mouse_position = GetMousePosition();
    Event event = {.mouse_hover = CheckCollisionPointRec(ui.mouse_position, ui.root->node.rect)};
    container_update((Node *)ui.root, event);
    if (IsWindowResized() || ui.update)
    {
        ui_resize(GetScreenWidth(), GetScreenHeight());
        ui.update = false;
    }
}

void ui_commit(void)
{
    ui.update = true;
}
