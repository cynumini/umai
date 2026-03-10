#include <raylib.h>
#include <sqlite3.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

static_assert(sizeof(int) == 4);
using i32 = int;
static_assert(sizeof(unsigned int) == 4);
using u32 = unsigned int;
static_assert(sizeof(unsigned int) == 4);
using u32 = unsigned int;
static_assert(sizeof(float) == 4);
using f32 = float;

using usize = size_t;

#define ASSERT(expr)                                                                                                   \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(expr))                                                                                                   \
        {                                                                                                              \
            fprintf(stderr, "%s:%i:0: Assertion \"" #expr "\" failed.\n", __FILE__, __LINE__);                         \
            abort();                                                                                                   \
        }                                                                                                              \
    } while (false)

#define UNREACHABLE                                                                                                    \
    do                                                                                                                 \
    {                                                                                                                  \
        fprintf(stderr, "%s:%i:0: Unreachable.\n", __FILE__, __LINE__);                                                \
        abort();                                                                                                       \
    } while (false)

namespace UI
{
namespace state
{
Vector2 offset;
bool temp_offset_enable = false;
Vector2 temp_offset;
f32 width;
f32 height;
bool same_line = false;
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

Rectangle calc_button_rectangle(const char *text, Vector2 offset = {0, 0})
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

void input(char *buffer, f32 width, const char *placeholder = "")
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

} // namespace UI

void handle_sqlite3_error_message(char *errmsg)
{
    printf("SQL: %s\n", errmsg);
    sqlite3_free(errmsg);
    UNREACHABLE;
}

struct Tab
{
    char name[128] = {};
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    InitWindow(1280, 720, "umai");

    constexpr usize tabs_capacity = 3;
    Tab tabs[tabs_capacity] = {};
    usize tabs_len = 0;

    usize active_tab_id = 0;

    const char *path = "database.db";
    bool is_fresh_start = false;
    {
        struct stat path_stat;
        is_fresh_start = stat(path, &path_stat) == 0 ? false : true;
    }

    sqlite3 *database;
    ASSERT(sqlite3_open(path, &database) == SQLITE_OK);
    const char *sql = "CREATE TABLE food ("
                      "id INTEGER NOT NULL UNIQUE,"
                      "created INTEGER NOT NULL,"
                      "name TEXT NOT NULL,"
                      "energy REAL NOT NULL,"
                      "PRIMARY KEY(id)"
                      ") STRICT;";
    {
        char *errmsg;
        if (is_fresh_start and sqlite3_exec(database, sql, nullptr, nullptr, &errmsg) != SQLITE_OK)
        {
            handle_sqlite3_error_message(errmsg);
        }
    }

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(LIGHTGRAY);
        UI::begin(GetScreenWidth(), GetScreenHeight());
        {
            if (UI::button("Add a food"))
            {
                tabs_len++;
                ASSERT(tabs_len <= tabs_capacity);
                active_tab_id = tabs_len;
            }
            UI::tab_container_begin(&active_tab_id);
            {
                UI::tab_begin("Main");
                {
                    UI::button("Main 1");
                }
                UI::tab_end();

                for (usize i = 0; i < tabs_len; i++)
                {
                    UI::tab_begin("Add a food");
                    {
                        UI::label("Name:");
                        UI::same_line();
                        UI::input(tabs[i].name, 256, "Food name");
                        auto add_rect = UI::calc_button_rectangle("Add");
                        auto cancel_rect = UI::calc_button_rectangle("Cancel");
                        f32 total_width = add_rect.width + cancel_rect.width + UI::style::child_gap * 2;
                        Vector2 offset = {
                            UI::state::width - total_width,
                            UI::state::height - (add_rect.height + UI::style::child_gap),
                        };
                        UI::temporarily_change_offset(offset);
                        if (UI::button("Add"))
                        {
                            int64_t created = time(NULL);
                            char sql[256];
                            sprintf(sql, "INSERT INTO food (created, name, energy) VALUES(%ld, \"%s\", %f)", created,
                                    tabs[i].name, 100.0);
                            {
                                char *errmsg;
                                if (sqlite3_exec(database, sql, NULL, NULL, &errmsg) != SQLITE_OK)
                                {
                                    handle_sqlite3_error_message(errmsg);
                                }
                            }
                        }
                        offset.x += add_rect.width + UI::style::child_gap;
                        UI::temporarily_change_offset(offset);
                        if (UI::button("Cancel"))
                        {
                            if (i == (tabs_len - 1))
                            {
                                tabs[i] = {};
                                tabs_len--;
                                active_tab_id--;
                            }
                            else
                            {
                                for (usize j = i + 1; j < tabs_len; j++)
                                {
                                    tabs[j - 1] = tabs[j];
                                }
                                tabs_len--;
                                tabs[tabs_len] = {};
                                active_tab_id--;
                            }
                        }
                    }
                    UI::tab_end();
                }
            }
        }
        EndDrawing();
    }

    sqlite3_close(database);
    CloseWindow();

    return 0;
}
