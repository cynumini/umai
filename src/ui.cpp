#include "ui.hpp"

#include <string.h>

namespace ui
{
namespace state
{
Vector2 offset;
bool temp_offset_enable = false;
Vector2 temp_offset;
f32 width;
f32 height;
bool same_line = false;
// table
constexpr usize table_cell_len = 64;
constexpr usize table_max_row = 32;
constexpr usize table_max_column = 4;
char table_data[table_max_row][table_max_column][table_cell_len] = {};
f32 table_cell_width[table_max_column] = {};
bool table_is_inside = false;
usize table_row_count = 0;
usize table_column_count = 0;
usize table_max_column_count = 0;
} // namespace state
namespace style
{
f32 padding = 2;
f32 child_gap = 2;
i32 font_size = 20;
} // namespace style
Vector2 mouse_position;
Rectangle previous;
Rectangle previous_tab;

usize current_tab_id = 0;
usize *current_active_tab_id = 0;
bool container_is_visible = true;

void same_line()
{
    if (container_is_visible)
    {
        state::same_line = true;
    }
}

Vector2 calc_local_offset()
{
    Vector2 local_offset;
    if (state::same_line)
    {
        local_offset = {previous.x + previous.width, previous.y};
    }
    else
    {
        local_offset = {state::offset.x, previous.y + previous.height};
    }
    return local_offset;
}

void temporarily_change_offset(Vector2 offset)
{
    state::temp_offset_enable = true;
    state::temp_offset = offset;
}

Rectangle calc_button_rectangle(const char *text, Vector2 offset)
{
    i32 width = MeasureText(text, 20);
    i32 height = 20;

    return {offset.x, offset.y, width + (style::padding * 2), height + (style::padding * 2)};
}

bool button(const char *text)
{
    if (!container_is_visible)
    {
        state::temp_offset_enable = false;
        return false;
    };

    Vector2 offset = state::temp_offset_enable ? state::temp_offset : calc_local_offset();

    Rectangle rectangle = calc_button_rectangle(text, offset);
    Color background = LIGHTGRAY;

    bool result = false;
    if (CheckCollisionPointRec(mouse_position, rectangle))
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            result = true;
        }

        background = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? DARKBLUE : BLUE;
    }
    DrawRectangleRec(rectangle, background);
    DrawRectangleLinesEx(rectangle, 1, BLACK);
    DrawText(text, offset.x + style::padding, offset.y + style::padding, style::font_size, BLACK);
    if (state::temp_offset_enable)
    {
        state::temp_offset_enable = false;
    }
    else
    {
        previous = rectangle;
    }
    state::same_line = false;
    return result;
}

void label(const char *text)
{
    if (state::table_is_inside == true)
    {
        using namespace state;
        usize row = table_row_count;
        usize column = table_column_count;
        assert(strlen(text) < table_cell_len);
        assert(row < table_max_row);
        assert(column < table_max_column);
        strcpy(table_data[row][column], text);
        table_column_count++;
        f32 width = MeasureText(text, 20);
        table_cell_width[column] = max(width, table_cell_width[column]);
    }
    else
    {
        if (!container_is_visible) return;

        Vector2 local_offset = calc_local_offset();

        f32 padding = 2;
        i32 width = MeasureText(text, 20);
        i32 height = 20;
        Rectangle rectangle = {local_offset.x, local_offset.y, width + (padding * 2), height + (padding * 2)};
        DrawText(text, local_offset.x + 2, local_offset.y + 2, height, BLACK);
        previous = rectangle;
        state::same_line = false;
    }
}

void input(char *buffer, f32 width, const char *placeholder)
{
    if (!container_is_visible) return;

    Vector2 local_offset = calc_local_offset();

    f32 padding = 2;
    i32 height = 20;
    Rectangle rectangle = {local_offset.x, local_offset.y, width + (padding * 2), height + (padding * 2)};
    DrawRectangleRec(rectangle, WHITE);
    DrawRectangleLinesEx(rectangle, 1, BLACK);
    usize buffer_len = strlen(buffer);
    if (buffer_len == 0)
    {
        if (strlen(placeholder))
        {
            DrawText(placeholder, local_offset.x + 2, local_offset.y + 2, height, GRAY);
        }
    }
    else
    {
        DrawText(buffer, local_offset.x + 2, local_offset.y + 2, height, BLACK);
    }

    bool active = true;
    if (active)
    {
        int c;
        while ((c = GetCharPressed()))
        {
            buffer[buffer_len] = c;
        }
        if (IsKeyPressed(KEY_BACKSPACE) && buffer_len)
        {
            buffer[buffer_len - 1] = 0;
        }
    }
    previous = rectangle;
    state::same_line = false;
}

void begin(f32 width, f32 height)
{
    state::offset = {};
    previous = {};
    state::width = width;
    state::height = height;
    mouse_position = GetMousePosition();
}

void tab_container_begin(usize *active_tab_id)
{
    f32 padding = 2;
    f32 tabs_panel_height = 20 + padding * 2;
    Vector2 local_offset = {state::offset.x, previous.y + previous.height};
    current_tab_id = 0;
    current_active_tab_id = active_tab_id;
    Rectangle rectangle = {
        local_offset.x,
        local_offset.y + tabs_panel_height,
        state::width,
        state::height - tabs_panel_height,
    };
    DrawRectangleRec(rectangle, WHITE);
    previous_tab = {};
    previous_tab.y = previous.y + previous.height;
}

void tab_begin(const char *name)
{
    f32 padding = 2;
    Rectangle rectangle = {
        previous_tab.x + previous_tab.width,
        previous_tab.y,
        (f32)MeasureText(name, 20) + padding * 2,
        20 + padding * 2,
    };

    if (*current_active_tab_id == current_tab_id)
    {
        DrawRectangleRec(rectangle, WHITE);
        container_is_visible = true;
    }
    else
    {
        Color background = GRAY;
        if (CheckCollisionPointRec(mouse_position, rectangle))
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                *current_active_tab_id = current_tab_id;
            }
            background = BLUE;
        }
        DrawRectangleRec(rectangle, background);
        container_is_visible = false;
    }

    DrawText(name, rectangle.x + padding, rectangle.y + padding, 20, BLACK);

    previous_tab = previous = rectangle;
    current_tab_id++;
}

void tab_end()
{
    container_is_visible = true;
}

void table_begin()
{
    state::table_row_count = 0;
    state::table_is_inside = true;
    memset(state::table_data, 0, sizeof(state::table_cell_width));
    memset(state::table_cell_width, 0, sizeof(state::table_cell_width));
}

void table_end()
{
    using namespace state;
    if (container_is_visible)
    {
        table_column_count = table_max_column_count;
        f32 line_thick = 1;
        Vector2 initial_offset = {offset.x, previous.y + previous.height};
        Rectangle rectangle = {
            initial_offset.x,
            initial_offset.y,
            (line_thick * (table_column_count + 1)) + (style::child_gap * (table_column_count * 2)),
            (line_thick * (table_row_count + 1)) + (style::child_gap * (table_row_count * 2)),
        };
        rectangle.height += table_row_count * style::font_size;
        for (usize column = 0; column < table_column_count; column++)
        {
            rectangle.width += table_cell_width[column];
        }
        Vector2 offset = {initial_offset.x + style::child_gap + line_thick,
                          initial_offset.y + style::child_gap + line_thick};
        for (usize row = 0; row < table_row_count; row++)
        {
            f32 offset_x = offset.x;
            for (usize column = 0; column < table_column_count; column++)
            {
                if (row == 0)
                {
                    f32 y = rectangle.y;
                    f32 x = offset_x + table_cell_width[column] + style::child_gap + line_thick;
                    DrawLine(x, y, x, y + rectangle.height, BLACK);
                }
                const char *text = table_data[row][column];
                DrawText(text, offset_x, offset.y, style::font_size, BLACK);
                offset_x += table_cell_width[column] + style::child_gap * 2 + line_thick;
            }
            f32 x = rectangle.x;
            f32 y = offset.y + style::font_size + style::child_gap;
            DrawLine(x, y, x + rectangle.width, y, BLACK);
            offset.y += style::font_size + style::child_gap * 2 + line_thick;
        }
        DrawRectangleLinesEx(rectangle, 1, BLACK);
        previous = rectangle;
    }
    table_is_inside = false;
}

void row_begin()
{
    state::table_column_count = 0;
}

void row_end()
{
    state::table_row_count++;
    state::table_max_column_count = max(state::table_max_column_count, state::table_column_count);
}

} // namespace ui
