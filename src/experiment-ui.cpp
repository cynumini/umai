#include "sakana.hpp"
#include "ui.hpp"

#include <math.h>
#include <string.h>

#ifndef STRING_LEN
#define STRING_LEN 128
#endif // !STRING_LEN

#define DRAW_QUEUE_CAPACITY 512
#define DRAW_COMMAND_CHILDREN_CAPACITY 512
#define MAX_VIEWPORT_CAPACITY 1

namespace ui
{
namespace style
{
i32 font_size = 20;
f32 padding = 8;
f32 child_gap = 8;
Color foreground = BLACK;
Color background = BLANK;
} // namespace style

Size size_fit()
{
    return {Size::fit};
}

Size size_grow()
{
    return {Size::grow};
}

Size size_fixed(f32 value)
{
    return {Size::fixed, value};
}

Sides sides_all(f32 value)
{
    return {value, value, value, value};
}

struct DrawCommand
{
    enum
    {
        container,
        scrollview,
        label,
    } type;
    Direction direction;
    Alignment alignment;
    usize parent_id;
    Size width;
    Size height;
    Sides paddings;
    f32 child_gap;
    f32 border_size;
    Color background;
    Rectangle rect = {};
    RenderTexture *viewport = nullptr;
    char text[STRING_LEN] = {};
    f32 *scroll = nullptr;
    enum
    {
        none,
        one,
        many
    } children_type;
    Array<usize, DRAW_COMMAND_CHILDREN_CAPACITY> children{};
};

static Array<DrawCommand, DRAW_QUEUE_CAPACITY> draw_queue{};

static usize current_id = 0;
static usize current_container_id = 0;
static RenderTexture viewports[MAX_VIEWPORT_CAPACITY] = {};
static usize current_viewport_id = 0;

static void draw_queue_add(DrawCommand command)
{
    draw_queue[current_container_id].children.add(draw_queue.len);
    draw_queue.add(command);
    current_id++;
}

void begin(f32 width, f32 height, RootContainerOptions options)
{
    DrawCommand command = {
        .type = DrawCommand::container,
        .direction = options.direction,
        .alignment = options.alignment,
        .parent_id = 0,
        .width = size_fixed(width),
        .height = size_fixed(height),
        .paddings = options.padding,
        .child_gap = style::child_gap,
        .border_size = 0,
        .background = options.background,
        .children_type = DrawCommand::many,
    };
    draw_queue.add(command);
}

static void fit_width(DrawCommand *command)
{
    for (auto id : command->children)
    {
        auto child = &draw_queue[id];
        fit_width(child);
    }

    if (command->width.type == Size::fixed)
    {
        command->rect.width = command->width.value;
    }
    else if (command->type == DrawCommand::container)
    {
        f32 self_width = command->paddings.left + command->paddings.right + (command->border_size * 2);
        if (command->direction == Direction::left_to_right)
        {
            f32 total_width = self_width;
            if (command->children.len > 0)
            {
                total_width += (command->children.len - 1) * command->child_gap;
            }
            for (auto id : command->children)
            {
                auto child = &draw_queue[id];
                total_width += child->rect.width;
            }
            command->rect.width = total_width;
        }
        else // top_to_bottom
        {
            f32 max_width = 0;
            for (auto id : command->children)
            {
                auto child = &draw_queue[id];
                max_width = max(child->rect.width, max_width);
            }
            command->rect.width = max_width + self_width;
        }
    }
    else if (command->type == DrawCommand::scrollview)
    {
        command->rect.width = 0;
    }
    else
    {
        unreachable();
    }
}

void fit_height(DrawCommand *command)
{
    for (auto id : command->children)
    {
        auto child = &draw_queue[id];
        fit_height(child);
    }

    if (command->height.type == Size::fixed)
    {
        command->rect.height = command->height.value;
    }
    else if (command->type == DrawCommand::container)
    {
        u32 self_height = command->paddings.top + command->paddings.bottom + (command->border_size * 2);
        if (command->direction == Direction::top_to_bottom)
        {
            f32 total_height = self_height;
            if (command->children.len > 0)
            {
                total_height += (command->children.len - 1) * command->child_gap;
            }
            for (auto id : command->children)
            {
                auto child = &draw_queue[id];
                total_height += child->rect.height;
            }
            command->rect.height = total_height;
        }
        else // left_to_right
        {
            f32 max_height = 0;
            for (auto id : command->children)
            {
                auto child = &draw_queue[id];
                max_height = max(child->rect.height, max_height);
            }
            command->rect.height = max_height + self_height;
        }
    }
    else if (command->type == DrawCommand::scrollview)
    {
        command->rect.height = 0;
    }
    else
    {
        unreachable();
    }
}

void grow_height(DrawCommand *command)
{
    f32 remaining_height = command->rect.height;
    remaining_height -= command->paddings.top + command->paddings.bottom + (command->border_size * 2);
    Array<DrawCommand *, DRAW_COMMAND_CHILDREN_CAPACITY> growable;

    if (command->direction == Direction::left_to_right)
    {
        for (usize id : command->children)
        {
            auto child = &draw_queue[id];
            if (child->height.type == Size::grow)
            {
                child->rect.height = max(remaining_height, child->rect.height);
            }
        }
    }
    else if (command->direction == Direction::top_to_bottom)
    {
        if (command->children.len > 0)
        {
            remaining_height -= command->child_gap * (command->children.len - 1);
        }

        for (usize id : command->children)
        {
            auto child = &draw_queue[id];
            remaining_height -= child->rect.height;
            if (child->height.type == Size::grow)
            {
                growable.add(child);
            }
        }

        while (remaining_height > 0 && growable.len > 0)
        {
            f32 smallest = growable.items[0]->rect.height;
            f32 second_smallest = INFINITY;
            f32 height_to_add = remaining_height;
            for (auto child : growable)
            {
                if (child->rect.height < smallest)
                {
                    second_smallest = smallest;
                    smallest = child->rect.height;
                }
                if (child->rect.height > smallest)
                {
                    second_smallest = min(second_smallest, child->rect.height);
                    height_to_add = second_smallest - smallest;
                }
            }
            height_to_add = min(height_to_add, remaining_height / growable.len);
            if (height_to_add == 0) break;
            for (auto child : growable)
            {
                if (child->rect.height == smallest)
                {
                    child->rect.height += height_to_add;
                    remaining_height -= height_to_add;
                }
            }
        }
    }

    for (usize id : command->children)
    {
        auto child = &draw_queue[id];
        if (child->type == DrawCommand::container)
        {
            grow_height(child);
        }
    }
}

void grow_width(DrawCommand *command)
{
    f32 remaining_width = command->rect.width;
    remaining_width -= command->paddings.left + command->paddings.right + (command->border_size * 2);
    Array<DrawCommand *, DRAW_COMMAND_CHILDREN_CAPACITY> growable;

    if (command->direction == Direction::left_to_right)
    {
        if (command->children.len > 0)
        {
            remaining_width -= command->child_gap * (command->children.len - 1);
        }

        for (usize id : command->children)
        {
            auto child = &draw_queue[id];
            remaining_width -= child->rect.width;
            if (child->width.type == Size::grow)
            {
                growable.add(child);
            }
        }

        while (remaining_width > 0 && growable.len > 0)
        {
            f32 smallest = growable.items[0]->rect.width;
            f32 second_smallest = INFINITY;
            f32 width_to_add = remaining_width;
            for (auto child : growable)
            {
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
            if (width_to_add == 0) break;
            for (auto child : growable)
            {
                if (child->rect.width == smallest)
                {
                    child->rect.width += width_to_add;
                    remaining_width -= width_to_add;
                }
            }
        }
    }
    else if (command->direction == Direction::top_to_bottom)
    {
        for (usize id : command->children)
        {
            auto child = &draw_queue[id];
            if (child->width.type == Size::grow)
            {
                child->rect.width = max(remaining_width, child->rect.width);
            }
        }
    }

    for (usize id : command->children)
    {
        auto child = &draw_queue[id];
        if (child->type == DrawCommand::container)
        {
            grow_width(child);
        }
    }
}

static void position(DrawCommand *command)
{

    f32 left_offset = command->rect.x + command->paddings.right + command->border_size;
    f32 top_offset = command->rect.y + command->paddings.top + command->border_size;

    f32 remaining_width = command->rect.width;
    f32 remaining_height = command->rect.width;

    for (auto id : command->children)
    {
        auto child = &draw_queue[id];
        remaining_width -= child->rect.width;
        remaining_height -= child->rect.height;
    }

    if (command->direction == Direction::left_to_right)
    {
        if (command->children.len > 0)
        {
            remaining_width -= (command->children.len - 1) * command->child_gap;
        }
        if (command->alignment == Alignment::center)
        {
            left_offset += remaining_width / 2;
        }
        for (auto id : command->children)
        {
            auto child = &draw_queue[id];
            Vector2 child_position = {};
            child_position.x += left_offset;
            child_position.y = top_offset;
            if (command->alignment == Alignment::center)
            {
                child_position.y += (command->rect.height - child->rect.height) / 2;
            }
            child->rect.x = child_position.x;
            child->rect.y = child_position.y;
            left_offset += child->rect.width + command->child_gap;
        }
    }
    else if (command->direction == Direction::top_to_bottom)
    {
        if (command->children.len > 0)
        {
            remaining_height -= (command->children.len - 1) * command->child_gap;
        }
        if (command->alignment == Alignment::center)
        {
            top_offset += remaining_height / 2;
        }
        for (auto id : command->children)
        {
            auto child = &draw_queue[id];
            Vector2 child_position = {};
            if (command->alignment == Alignment::center)
            {
                child_position.x = (command->rect.width - child->rect.width) / 2;
            }
            else
            {
                child_position.x += left_offset;
            }
            child_position.y += top_offset;
            child->rect.x = child_position.x;
            child->rect.y = child_position.y;
            top_offset += child->rect.height + command->child_gap;
        }
    }

    for (auto id : command->children)
    {
        auto child = &draw_queue[id];
        if (child->type == DrawCommand::container)
        {
            position(child);
        }
    }
}

static void exec_draw_command(DrawCommand *command)
{
    switch (command->type)
    {
    case DrawCommand::container: {
        DrawRectangleRec(command->rect, command->background);
        for (usize id : command->children)
        {
            auto child = &draw_queue[id];
            exec_draw_command(child);
        }
    }
    break;
    case DrawCommand::label: {
        DrawText(command->text, command->rect.x, command->rect.y, style::font_size, style::foreground);
    }
    break;
    case DrawCommand::scrollview:
        if (command->viewport)
        {
            f32 diff = command->viewport->texture.height - command->rect.height;
            // f32 height = -command->rect.height;
            DrawTextureRec(command->viewport->texture, {0, diff, command->rect.width, -command->rect.height},
                           {command->rect.x, command->rect.y}, WHITE);
        }
        break;
    }
}

static void calc_layout(DrawCommand *command)
{
    fit_width(command);
    grow_width(command);
    fit_height(command);
    grow_height(command);
    position(command);
}

void end()
{
    calc_layout(&draw_queue[0]);
    for (usize i = draw_queue.len; i > 0; i--)
    {
        auto child = &draw_queue[i - 1];
        if (child->type == DrawCommand::scrollview)
        {
            auto root = &draw_queue[child->children[0]];
            auto viewport = &viewports[current_viewport_id];
            auto width = child->rect.width, height = child->rect.height;
            if (width > viewport->texture.width or height > viewport->texture.height)
            {
                UnloadRenderTexture(*viewport);
                *viewport = LoadRenderTexture(width, height);
            }
            Vector2 temp = {root->rect.x, root->rect.y};
            *child->scroll = min(*child->scroll, 0.f);
            *child->scroll = max(*child->scroll, -root->rect.height + child->rect.height);
            root->rect.x = 0;
            root->rect.y = *child->scroll;
            // root->rect.y = child->scroll;
            BeginTextureMode(*viewport);
            ClearBackground(WHITE);
            calc_layout(root);
            exec_draw_command(root);
            EndTextureMode();
            root->rect.x = temp.x;
            root->rect.y = temp.y;
            child->viewport = viewport;
        }
    }
}

void container_begin(ContainerOptions options)
{
    DrawCommand command = {
        .type = DrawCommand::container,
        .direction = options.direction,
        .alignment = options.alignment,
        .parent_id = current_container_id,
        .width = options.width,
        .height = options.height,
        .paddings = options.padding,
        .child_gap = style::child_gap,
        .border_size = options.border_size,
        .background = options.background,
        .children_type = DrawCommand::many,
    };
    draw_queue_add(command);
    current_container_id = current_id;
}

void container_end()
{
    current_container_id = draw_queue[current_container_id].parent_id;
}

void scrollview_begin(f32 *scroll, ScrollViewOptions options)
{
    DrawCommand command = {
        .type = DrawCommand::scrollview,
        .direction = Direction::left_to_right,
        .alignment = Alignment::left_top,
        .parent_id = current_container_id,
        .width = options.width,
        .height = options.height,
        .paddings = options.padding,
        .child_gap = 0,
        .border_size = options.border_size,
        .background = options.background,
        .children_type = DrawCommand::one,
    };
    if (GetMouseWheelMove() > 0)
    {
        *scroll += 128;
    }
    if (GetMouseWheelMove() < 0)
    {
        *scroll -= 128;
    }
    command.scroll = scroll;
    // printf("%f\n", command.scroll);
    draw_queue_add(command);
    current_container_id = current_id;
}

void scrollview_end()
{
    current_container_id = draw_queue[current_container_id].parent_id;
}

void label(const char *text, LabelOptions options)
{
    f32 width = MeasureText(text, style::font_size), height = style::font_size;
    DrawCommand command = {
        .type = DrawCommand::label,
        .direction = Direction::left_to_right,
        .alignment = Alignment::left_top,
        .parent_id = current_container_id,
        .width = size_fixed(width),
        .height = size_fixed(height),
        .paddings = options.padding,
        .child_gap = 0,
        .border_size = 0,
        .background = BLANK,
        .children_type = DrawCommand::none,
    };
    assert(strlen(text) < STRING_LEN);
    strcpy(command.text, text);
    draw_queue_add(command);
}

void draw()
{
    exec_draw_command(&draw_queue[0]);
    draw_queue.reset();
    current_id = 0;
    current_viewport_id = 0;
}
} // namespace ui
