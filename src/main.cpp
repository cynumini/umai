#include <raylib.h>
#include <sqlite3.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "sakana.hpp"
#include "ui.hpp"

using namespace sakana;

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
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "umai");

    sqlite3 *database;
    bool database_need_update = true;

    constexpr usize foods_max_len = 32;
    [[maybe_unused]] usize foods_len = 0;
    Food foods[foods_max_len];

    auto database_exec = [&database](const char *sql) {
        char *errmsg;
        if (sqlite3_exec(database, sql, nullptr, nullptr, &errmsg) != SQLITE_OK)
        {
            printf("SQL: %s\n", errmsg);
            sqlite3_free(errmsg);
            unreachable();
        }
    };
    {
        const char *path = "database.db";
        struct stat path_stat;
        bool is_fresh_start = stat(path, &path_stat) == 0 ? false : true;
        assert(sqlite3_open(path, &database) == SQLITE_OK);
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

    // scrollview

    RenderTexture2D scrollview = LoadRenderTexture(500, 500);

    while (!WindowShouldClose())
    {
        f32 width = GetScreenWidth();
        f32 height = GetScreenHeight();

        BeginTextureMode(scrollview);
        ClearBackground(RAYWHITE);
        DrawRectangle(0, 0, 32, 32, BLUE);
        DrawText("TEST", 20, 20, 20, RED);
        EndTextureMode();
        if (database_need_update)
        {
            memset(foods, 0, sizeof(foods));

            sqlite3_stmt *stmt = nullptr;
            const char *sql = "SELECT * FROM food";
            int rc = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
            assert(rc == SQLITE_OK);
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
                            unreachable();
                        }
                        break;
                    }
                    case SQLITE_TEXT: {
                        const unsigned char *value = sqlite3_column_text(stmt, i);
                        if (strcmp(name, "name") == 0)
                        {
                            const char *name = (const char *)value;
                            assert(strlen(name) < food_name_max_strlen);
                            strcpy(food.name, name);
                        }
                        else
                        {
                            printf("%s(TEXT) = %s\n", name, value);
                            unreachable();
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
                            unreachable();
                        }
                        break;
                    }
                    default: {
                        unreachable();
                        break;
                    }
                    }
                }
                assert(i < foods_max_len);
                foods[i] = food;
                i++;
                foods_len = i;
                rc = sqlite3_step(stmt);
            }
            rc = sqlite3_finalize(stmt);
        }
        BeginDrawing();
        ClearBackground(LIGHTGRAY);
        ui::begin(width, height);
        {
            if (ui::button("Add a food"))
            {
                tabs_len++;
                assert(tabs_len <= tabs_capacity);
                tabs_active = tabs_len;
            }
            ui::tab_container_begin(&tabs_active);
            {
                ui::tab_begin("Main");
                {
                    // ui::scrollview_begin();
                    ui::table_begin();
                    {
                        for (usize i = 0; i < foods_len; i++)
                        {
                            ui::row_begin();
                            char buffer[64];
                            sprintf(buffer, "%li", foods[i].id);
                            ui::label(buffer);
                            sprintf(buffer, "%li", foods[i].created);
                            ui::label(buffer);
                            ui::label(foods[i].name);
                            sprintf(buffer, "%f", foods[i].energy);
                            ui::label(buffer);
                            ui::row_end();
                        }
                    }
                    ui::table_end();
                    // ui::scrollview_end()
                }
                ui::tab_end();

                for (usize i = 0; i < tabs_len; i++)
                {
                    ui::tab_begin("Add a food");
                    {
                        ui::label("Name:");
                        ui::same_line();
                        ui::input(tabs[i].name, 256, "Food name");
                        auto add_rect = ui::calc_button_rectangle("Add");
                        auto cancel_rect = ui::calc_button_rectangle("Cancel");
                        f32 total_width = add_rect.width + cancel_rect.width + ui::style::child_gap * 2;
                        Vector2 offset = {
                            width - total_width,
                            height - (add_rect.height + ui::style::child_gap),
                        };
                        ui::temporarily_change_offset(offset);
                        if (ui::button("Add"))
                        {
                            int64_t created = time(NULL);
                            char sql[256];
                            sprintf(sql, "INSERT INTO food (created, name, energy) VALUES(%ld, \"%s\", %f)", created,
                                    tabs[i].name, 100.0);
                            database_exec(sql);
                            close_tab(i);
                        }
                        offset.x += add_rect.width + ui::style::child_gap;
                        ui::temporarily_change_offset(offset);
                        if (ui::button("Cancel"))
                        {
                            close_tab(i);
                        }
                        database_need_update = true;
                    }
                    ui::tab_end();
                }
            }
        }
        EndDrawing();
    }

    UnloadRenderTexture(scrollview);

    sqlite3_close(database);
    CloseWindow();

    return 0;
}
