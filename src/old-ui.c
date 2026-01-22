#include "old-ui.h"

#include <math.h>
#include <stdio.h>

#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

// Size
Size size_fixed(int value)
{
    return (Size){SIZE_TYPE_FIXED, value};
}

Size size_grow(void)
{
    return (Size){SIZE_TYPE_GROW, 0};
}

Size size_fit(void)
{
    return (Size){SIZE_TYPE_FIT, 0};
}

// Padding
Padding padding_all(int value)
{
    return (Padding){value, value, value, value};
}

// Element
static void element_fit_width(OldNode *root)
{
    OldElement *self = (OldElement *)root;
    float min_width = 0;
    if (self->layout_direction == LAYOUT_DIRECTION_LEFT_TO_RIGHT)
    {
        float child_gap = self->children.len > 0 ? self->child_gap * (self->children.len - 1) : 0;
        min_width = self->padding.left + self->padding.right + child_gap;
        for (size_t i = 0; i < self->children.len; i++)
        {
            OldNode *child = self->children.items[i];
            child->calc_fit_width(child);
            min_width += child->rect.width;
        }
    }
    else
    {
        min_width = self->padding.left + self->padding.right;
        float child_max_min_width = 0;
        for (size_t i = 0; i < self->children.len; i++)
        {
            OldNode *child = self->children.items[i];
            child->calc_fit_width(child);
            child_max_min_width = max_float(child->rect.width, child_max_min_width);
        }
        min_width += child_max_min_width;
    }
    if (root->width.type == SIZE_TYPE_FIXED)
    {
        min_width = root->width.value;
    }
    root->rect.width = min_width;
}

static void element_fit_height(OldNode *root)
{
    OldElement *self = (OldElement *)root;
    float min_height = 0;
    if (self->layout_direction == LAYOUT_DIRECTION_LEFT_TO_RIGHT)
    {
        min_height = self->padding.top + self->padding.bottom;
        int child_max_min_height = 0;
        for (size_t i = 0; i < self->children.len; i++)
        {
            OldNode *child = self->children.items[i];
            child->calc_fit_height(child);
            child_max_min_height = max_float(child->rect.height, child_max_min_height);
        }
        min_height += child_max_min_height;
    }
    else
    {
        float child_gap = self->children.len > 0 ? self->child_gap * (self->children.len - 1) : 0;
        min_height = self->padding.top + self->padding.bottom + child_gap;
        for (size_t i = 0; i < self->children.len; i++)
        {
            OldNode *child = self->children.items[i];
            child->calc_fit_height(child);
            min_height += child->rect.height;
        }
    }
    if (root->height.type == SIZE_TYPE_FIXED)
    {
        min_height = root->height.value;
    }
    root->rect.height = min_height;
}

static void element_calc_grow_width(OldNode *root)
{
    OldElement *self = (OldElement *)root;
    float remaining_width = root->rect.width;
    remaining_width -= self->padding.left + self->padding.right;
    NodePointerArray growable = {0};
    if (self->layout_direction == LAYOUT_DIRECTION_LEFT_TO_RIGHT)
    {
        remaining_width -= self->child_gap * (self->children.len - 1);
        for (size_t i = 0; i < self->children.len; i++)
        {
            OldNode *child = self->children.items[i];
            remaining_width -= child->rect.width;
            if (child->width.type == SIZE_TYPE_GROW)
            {
                DYNAMIC_ARRAY_ADD(&growable, child);
            }
        }
        while (remaining_width > 0 && growable.len > 0)
        {
            float smallest = growable.items[0]->rect.width;
            float second_smallest = INFINITY;
            float width_to_add = remaining_width;
            for (size_t i = 0; i < growable.len; i++)
            {
                OldNode *child = growable.items[i];
                if (child->rect.width < smallest)
                {
                    second_smallest = smallest;
                    smallest = child->rect.width;
                }
                if (child->rect.width > smallest)
                {
                    second_smallest = min_float(second_smallest, child->rect.width);
                    width_to_add = second_smallest - smallest;
                }
            }
            width_to_add = min_float(width_to_add, remaining_width / growable.len);
            if (width_to_add == 0)
                break;
            for (size_t i = 0; i < growable.len; i++)
            {
                OldNode *child = growable.items[i];
                if (child->rect.width == smallest)
                {
                    child->rect.width += width_to_add;
                    remaining_width -= width_to_add;
                }
            }
        }
    }
    else if (self->layout_direction == LAYOUT_DIRECTION_TOP_TO_BOTTOM)
    {
        for (size_t i = 0; i < self->children.len; i++)
        {
            OldNode *child = self->children.items[i];
            if (child->width.type == SIZE_TYPE_GROW)
            {
                child->rect.width = remaining_width;
            }
        }
    }

    DYNAMIC_ARRAY_DEINIT(&growable);

    for (size_t i = 0; i < self->children.len; i++)
    {
        OldNode *child = self->children.items[i];
        if (child->calc_grow_width != NULL)
        {
            child->calc_grow_width(child);
        }
    }
}

static void element_calc_grow_height(OldNode *root)
{
    OldElement *self = (OldElement *)root;
    float remaining_height = root->rect.height;
    remaining_height -= self->padding.top + self->padding.bottom;
    NodePointerArray growable = {0};
    if (self->layout_direction == LAYOUT_DIRECTION_LEFT_TO_RIGHT)
    {
        for (size_t i = 0; i < self->children.len; i++)
        {
            OldNode *child = self->children.items[i];
            if (child->height.type == SIZE_TYPE_GROW)
            {
                child->rect.height = remaining_height;
            }
        }
    }
    else if (self->layout_direction == LAYOUT_DIRECTION_TOP_TO_BOTTOM)
    {
        remaining_height -= self->child_gap * (self->children.len - 1);
        for (size_t i = 0; i < self->children.len; i++)
        {
            OldNode *child = self->children.items[i];
            remaining_height -= child->rect.height;
            if (child->height.type == SIZE_TYPE_GROW)
            {
                DYNAMIC_ARRAY_ADD(&growable, child);
            }
        }
        while (remaining_height > 0 && growable.len > 0)
        {
            float smallest = growable.items[0]->rect.height;
            float second_smallest = INFINITY;
            float height_to_add = remaining_height;
            for (size_t i = 0; i < growable.len; i++)
            {
                OldNode *child = growable.items[i];
                if (child->rect.height < smallest)
                {
                    second_smallest = smallest;
                    smallest = child->rect.height;
                }
                if (child->rect.height > smallest)
                {
                    second_smallest = min_float(second_smallest, child->rect.height);
                    height_to_add = second_smallest - smallest;
                }
            }
            height_to_add = min_float(height_to_add, remaining_height / growable.len);
            if (height_to_add == 0)
                break;
            for (size_t i = 0; i < growable.len; i++)
            {
                OldNode *child = growable.items[i];
                if (child->rect.height == smallest)
                {
                    child->rect.height += height_to_add;
                    remaining_height -= height_to_add;
                }
            }
        }
    }

    DYNAMIC_ARRAY_DEINIT(&growable);

    for (size_t i = 0; i < self->children.len; i++)
    {
        OldNode *child = self->children.items[i];
        if (child->calc_grow_height != NULL)
        {
            child->calc_grow_height(child);
        }
    }
}

static void element_calc_position(OldNode *root)
{
    const OldElement *self = (const OldElement *)root;

    float left_offset = root->rect.x + self->padding.left;
    float top_offset = root->rect.y + self->padding.top;

    float remaining_width = root->rect.width;
    float remaining_height = root->rect.width;

    for (size_t i = 0; i < self->children.len; i++)
    {
        OldNode *child = self->children.items[i];
        remaining_width -= child->rect.width;
        remaining_height -= child->rect.height;
    }

    if (self->layout_direction == LAYOUT_DIRECTION_LEFT_TO_RIGHT)
    {
        if (root->alignment == ALIGNMENT_CENTER)
        {
            left_offset += remaining_width / 2;
        }
        for (size_t i = 0; i < self->children.len; i++)
        {
            OldNode *child = self->children.items[i];
            Vector2 child_position = Vector2Add(root->position, child->position);
            child_position.x += left_offset;
            if (root->alignment == ALIGNMENT_CENTER)
            {
                child_position.y = (root->rect.height - child->rect.height) / 2;
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
    else if (self->layout_direction == LAYOUT_DIRECTION_TOP_TO_BOTTOM)
    {
        if (root->alignment == ALIGNMENT_CENTER)
        {
            top_offset += remaining_height / 2;
        }
        for (size_t i = 0; i < self->children.len; i++)
        {
            OldNode *child = self->children.items[i];
            Vector2 child_position = Vector2Add(root->position, child->position);
            if (root->alignment == ALIGNMENT_CENTER)
            {
                child_position.x = (root->rect.width - child->rect.width) / 2;
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

    for (size_t i = 0; i < self->children.len; i++)
    {
        OldNode *child = self->children.items[i];
        if (child->calc_position != NULL)
        {
            child->calc_position(child);
        }
    }
}

static void element_calc_layout(OldNode *node)
{
    element_fit_width(node);
    element_calc_grow_width(node);
    element_fit_height(node);
    element_calc_grow_height(node);
    element_calc_position(node);
}

static void element_update(OldNode *node)
{
    OldElement *self = (OldElement *)node;
    for (size_t i = 0; i < self->children.len; i++)
    {
        OldNode *child = self->children.items[i];
        child->update(child);
    }
}

void element_draw(const OldNode *root)
{
    const OldElement *self = (const OldElement *)root;
    DrawRectangleRec(root->rect, root->color);
    for (size_t i = 0; i < self->children.len; i++)
    {
        OldNode *child = self->children.items[i];
        child->draw(child);
    }
}

OldElement element_init(void)
{
    return (OldElement){
        .node =
            {
                .position = {0},
                .rect = {0},
                .color = BLANK,
                .width = {0},
                .height = {0},
                .calc_fit_width = element_fit_width,
                .calc_fit_height = element_fit_height,
                .calc_grow_width = element_calc_grow_width,
                .calc_grow_height = element_calc_grow_height,
                .calc_position = element_calc_position,
                .calc_layout = element_calc_layout,
                .update = element_update,
                .draw = element_draw,
            },
        .padding = {0},
        .layout_direction = LAYOUT_DIRECTION_LEFT_TO_RIGHT,
        .child_gap = 0,
        .children = {0},
    };
}

void element_add_child(OldElement *self, OldNode *node)
{
    DYNAMIC_ARRAY_ADD(&self->children, node);
}

void element_deinit(OldElement *self)
{
    DYNAMIC_ARRAY_DEINIT(&self->children);
}

// Text
static void text_calc_fit_width(OldNode *node)
{
    Text *self = (Text *)node;

    if (self->wrap)
    {
        char *text = strdup(self->text);
        const char *token = strtok(text, " ");
        float min_width = 0;
        while (token != NULL)
        {
            min_width = max_int(MeasureText(token, 20), min_width);
            token = strtok(NULL, " ");
        }
        node->rect.width = min_width;
        free(text);
    }
    else
    {
        node->rect.width = MeasureText(self->text, 20);
    }
}

static void text_calc_fit_height(OldNode *node)
{
    Text *self = (Text *)node;

    if (self->wrap)
    {
        if (self->lines.len > 0)
        {
            DYNAMIC_ARRAY_DEINIT(&self->lines);
            self->lines = (StringArray){0};
        }
        char *text = strdup(self->text);
        const char *token = strtok(text, " ");
        char *buffer = NULL;
        while (token != NULL)
        {
            if (buffer == NULL)
            {
                buffer = strdup(token);
            }
            else
            {
                char *tmp = strdup(buffer);
                buffer = realloc(buffer, sizeof(char) + strlen(tmp) + strlen(token) + 2);
                sprintf(buffer, "%s %s", tmp, token);

                if (MeasureText(buffer, 20) > node->rect.width)
                {
                    DYNAMIC_ARRAY_ADD(&self->lines, tmp);
                    buffer = strdup(token);
                }
                else
                {
                    free(tmp);
                }
            }

            token = strtok(NULL, " ");
        }
        DYNAMIC_ARRAY_ADD(&self->lines, buffer);
        free(text);
        node->rect.height = 20 * self->lines.len;
    }
    else
    {
        node->rect.height = 20;
    }
}

static void text_update(OldNode *node)
{
    (void)node;
}

static void text_draw(const OldNode *node)
{
    const Text *self = (const Text *)node;
    if (self->wrap)
    {
        float y_offset = node->rect.y;
        for (size_t i = 0; i < self->lines.len; i++)
        {
            DrawText(self->lines.items[i], node->rect.x, y_offset, 20, node->color);
            y_offset += 20;
        }
    }
    else
    {
        DrawText(self->text, node->rect.x, node->rect.y, 20, node->color);
    }
}

Text text_init(const char *text, bool wrap)
{
    return (Text){
        .node =
            {
                .position = {0},
                .rect = {0},
                .width = wrap ? size_grow() : size_fit(),
                .color = BLACK,
                .calc_fit_width = text_calc_fit_width,
                .calc_fit_height = text_calc_fit_height,
                .update = text_update,
                .draw = text_draw,
            },
        .text = text,
        .wrap = wrap,
    };
}

void text_deinit(Text *self)
{
    for (size_t i = 0; i < self->lines.len; i++)
    {
        free((void *)self->lines.items[i]);
    }
    DYNAMIC_ARRAY_DEINIT(&self->lines);
}

// Table
static void table_calc_fit_width(OldNode *node)
{
    const Table *self = (const Table *)node;
    void *data = self->data.data;
    const size_t row_count = self->data.get_row_count(data);
    const size_t column_count = self->data.get_column_count(data);
    node->rect.width = column_count * 2 + (column_count + 1);
    for (size_t i = 0; i < column_count; i++)
    {
        float max_column_width = 0;
        for (size_t j = 0; j < row_count; j++)
        {
            char buffer[256] = {0};
            self->data.get_cell(data, j, i, buffer);
            max_column_width = max_float(MeasureText(buffer, 20), max_column_width);
        }
        node->rect.width += max_column_width;
    }
}

static void table_calc_fit_height(OldNode *node)
{
    const Table *self = (const Table *)node;
    void *data = self->data.data;
    const size_t row_count = self->data.get_row_count(data);
    node->rect.height = row_count * 2 + (row_count + 1);
    node->rect.height += row_count * 20;
}

static void table_draw(OldNode const *node)
{
    const Table *self = (const Table *)node;
    void *data = self->data.data;
    const size_t row_count = self->data.get_row_count(data);
    const size_t column_count = self->data.get_column_count(data);

    if (self->selected_column != -1)
    {
        DrawRectangleRec(
            (Rectangle){
                node->rect.x,
                node->rect.y + self->selected_column * 23,
                node->rect.width,
                23,
            },
            BLUE);
    }

    int x_offset = node->rect.x + 2;

    for (size_t i = 0; i < column_count; i++)
    {
        float max_column_width = 0;
        int y_offset = node->rect.y + 2;
        for (size_t j = 0; j < row_count; j++)
        {
            if (i == 0)
            {
                float y = y_offset + 22;
                DrawLine(node->rect.x, y, node->rect.x + node->rect.width, y, BLACK);
            }
            char buffer[256] = {0};
            self->data.get_cell(data, j, i, buffer);
            DrawText(buffer, x_offset, y_offset, 20, BLACK);
            y_offset += 23;
            max_column_width = max_float(MeasureText(buffer, 20), max_column_width);
        }
        float x = x_offset + max_column_width + 2;
        DrawLine(x, node->rect.y, x, node->rect.y + node->rect.height, BLACK);
        x_offset += max_column_width + 3;
    }

    DrawRectangleLinesEx(node->rect, 1, BLACK);
}

static void table_update(OldNode *node)
{
    Table *self = (Table *)node;
    (void)self;

    Vector2 mouse_position = GetMousePosition();
    Vector2 relative = Vector2Subtract(mouse_position, (Vector2){node->rect.x, node->rect.y});
    bool is_mouse_over = CheckCollisionPointRec(mouse_position, node->rect);
    if (is_mouse_over)
    {
        if (IsKeyReleased(KEY_ESCAPE))
        {
            self->selected_column = -1;
        }
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            self->selected_column = (int)relative.y / 23;
            if (self->on_row_selected != NULL)
            {
                self->on_row_selected(self, self->selected_column, self->user_data);
            }
        }
    }
}

Table table_init(TableData data)
{
    return (Table){.node =
                       {
                           .calc_fit_width = table_calc_fit_width,
                           .calc_fit_height = table_calc_fit_height,
                           .update = table_update,
                           .draw = table_draw,
                       },
                   .data = data,
                   .selected_column = -1};
}

void table_deinit(Table *self)
{
    (void)self;
}

// TextInput
static void text_input_calc_fit_width(OldNode *node)
{
    (void)node;
}

TextInput text_input_init(void)
{
    return (TextInput){.node =
                           {
                               .calc_fit_width = text_input_calc_fit_width,
                               .draw = NULL,
                           },
                       .text = {0}};
}

void text_input_deinit(TextInput *self)
{
    (void)self;
}
