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
static_assert(sizeof(long) == 8);
using i64 = long;
static_assert(sizeof(unsigned int) == 4);
using u32 = unsigned int;
static_assert(sizeof(unsigned long) == 8);
using u64 = long;
static_assert(sizeof(float) == 4);
using f32 = float;
static_assert(sizeof(double) == 8);
using f64 = double;

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

template <typename T> T max(T a, T b)
{
    return a > b ? a : b;
}

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
// table
constexpr usize table_cell_len = 64;
constexpr usize table_max_row = 32;
constexpr usize table_max_column = 4;
char table_data[table_max_row][table_max_column][table_cell_len] = {};
f32 table_cell_width[table_max_column] = {};
bool table_is_inside = true;
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
    if (state::table_is_inside == true)
    {
        using namespace state;
        usize row = table_row_count;
        usize column = table_column_count;
        ASSERT(strlen(text) < table_cell_len);
        ASSERT(row < table_max_row);
        ASSERT(column < table_max_column);
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

} // namespace UI

struct Tab
{
    char name[128] = {};
};

constexpr usize food_name_max_strlen = 64;
struct Food
{
    i64 id;
    i64 created;
    char name[food_name_max_strlen];
    f32 energy;
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    InitWindow(1280, 720, "umai");

    sqlite3 *database;
    bool database_need_update = true;

    constexpr usize foods_max_len = 32;
    usize foods_len = 0;
    Food foods[foods_max_len];

    auto database_exec = [&database](const char *sql) {
        char *errmsg;
        if (sqlite3_exec(database, sql, nullptr, nullptr, &errmsg) != SQLITE_OK)
        {
            printf("SQL: %s\n", errmsg);
            sqlite3_free(errmsg);
            UNREACHABLE;
        }
    };
    {
        const char *path = "database.db";
        struct stat path_stat;
        bool is_fresh_start = stat(path, &path_stat) == 0 ? false : true;
        ASSERT(sqlite3_open(path, &database) == SQLITE_OK);
        const char *sql = "CREATE TABLE food ("
                          "id INTEGER NOT NULL UNIQUE,"
                          "created INTEGER NOT NULL,"
                          "name TEXT NOT NULL,"
                          "energy REAL NOT NULL,"
                          "PRIMARY KEY(id)"
                          ") STRICT;";
        if (is_fresh_start) database_exec(sql);
    }

    // Setup tabs
    constexpr usize tabs_capacity = 3;
    Tab tabs[tabs_capacity] = {};
    usize tabs_len = 0;
    usize tabs_active = 0;
    auto close_tab = [&tabs, &tabs_len, &tabs_active](usize index) {
        if (index == tabs_len - 1)
        {
            tabs[index] = {};
            tabs_len--;
            tabs_active--;
        }
        else
        {
            for (usize i = index + 1; i < tabs_len; i++)
            {
                tabs[i - 1] = tabs[i];
            }
            tabs_len--;
            tabs_active--;
            tabs[tabs_len] = {};
        }
    };

    while (!WindowShouldClose())
    {
        if (database_need_update)
        {
            memset(foods, 0, sizeof(foods));

            sqlite3_stmt *stmt = nullptr;
            const char *sql = "SELECT * FROM food";
            int rc = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
            ASSERT(rc == SQLITE_OK);
            rc = sqlite3_step(stmt);
            usize i = 0;
            while (rc != SQLITE_DONE)
            {
                Food food;
                int column_count = sqlite3_column_count(stmt);
                for (int i = 0; i < column_count; i++)
                {
                    int type = sqlite3_column_type(stmt, i);
                    const char *name = sqlite3_column_name(stmt, i);
                    switch (type)
                    {
                    case SQLITE_INTEGER: {
                        int64_t value = sqlite3_column_int64(stmt, i);
                        if (strcmp(name, "id") == 0)
                        {
                            food.id = value;
                        }
                        else if (strcmp(name, "created") == 0)
                        {
                            food.created = value;
                        }
                        else
                        {
                            printf("%s(INTEGER) = %li\n", name, value);
                            UNREACHABLE;
                        }
                        break;
                    }
                    case SQLITE_TEXT: {
                        const unsigned char *value = sqlite3_column_text(stmt, i);
                        if (strcmp(name, "name") == 0)
                        {
                            const char *name = (const char *)value;
                            ASSERT(strlen(name) < food_name_max_strlen);
                            strcpy(food.name, name);
                        }
                        else
                        {
                            printf("%s(TEXT) = %s\n", name, value);
                            UNREACHABLE;
                        }
                        break;
                    }
                    case SQLITE_FLOAT: {
                        double value = sqlite3_column_double(stmt, i);
                        if (strcmp(name, "energy") == 0)
                        {
                            food.energy = (float)value;
                        }
                        else
                        {
                            printf("%s(FLOAT) = %f\n", name, value);
                            UNREACHABLE;
                        }
                        break;
                    }
                    default: {
                        UNREACHABLE;
                        break;
                    }
                    }
                }
                ASSERT(i < foods_max_len);
                foods[i] = food;
                i++;
                foods_len = i;
                rc = sqlite3_step(stmt);
            }
            rc = sqlite3_finalize(stmt);
        }
        BeginDrawing();
        ClearBackground(LIGHTGRAY);
        UI::begin(GetScreenWidth(), GetScreenHeight());
        {
            if (UI::button("Add a food"))
            {
                tabs_len++;
                ASSERT(tabs_len <= tabs_capacity);
                tabs_active = tabs_len;
            }
            UI::tab_container_begin(&tabs_active);
            {
                UI::tab_begin("Main");
                {
                    UI::table_begin();
                    {
                        for (usize i = 0; i < foods_len; i++)
                        {
                            UI::row_begin();
                            char buffer[64];
                            sprintf(buffer, "%li", foods[i].id);
                            UI::label(buffer);
                            sprintf(buffer, "%li", foods[i].created);
                            UI::label(buffer);
                            UI::label(foods[i].name);
                            sprintf(buffer, "%f", foods[i].energy);
                            UI::label(buffer);
                            UI::row_end();
                        }
                    }
                    UI::table_end();
                    UI::label("A");
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
                            database_exec(sql);
                            close_tab(i);
                        }
                        offset.x += add_rect.width + UI::style::child_gap;
                        UI::temporarily_change_offset(offset);
                        if (UI::button("Cancel"))
                        {
                            close_tab(i);
                        }
                        database_need_update = true;
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
