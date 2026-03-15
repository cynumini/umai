#include <raylib.h>
#include <stdio.h>

#include "sakana.hpp"

#define STRING_LEN 32
#include "ui.hpp"

using namespace sakana;

#define printf_to_buffer(buffer, format, ...)                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        i32 len = snprintf(buffer, STRING_LEN, format, __VA_ARGS__);                                                   \
        assert(len < STRING_LEN);                                                                                      \
    } while (false)

i32 main([[maybe_unused]] i32 argc, [[maybe_unused]] char *argv[])
{
    i32 width = 1280, height = 720;
    f32 scroll = 0;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, "umai");

    while (!WindowShouldClose())
    {
        // update
        if (IsWindowResized())
        {
            width = GetScreenWidth();
            height = GetScreenHeight();
        }
        ui::begin(width, height, {.background = RED});
        ui::container_begin({
            .background = GREEN,
            .width = ui::size_grow(),
            .height = ui::size_grow(),
        });
        {
            ui::label("Yes, I am!");
            ui::container_begin({
                .background = BLUE,
                .width = ui::size_fixed(100),
                .height = ui::size_fixed(100),
            });
            ui::container_end();
            ui::scrollview_begin(&scroll, {.background = WHITE, .width = ui::size_grow(), .height = ui::size_grow()});
            {
                ui::container_begin({.direction = ui::Direction::top_to_bottom});
                for (usize i = 0; i < 500; i++)
                {
                    static char buffer[STRING_LEN] = {};
                    printf_to_buffer(buffer, "Hello, number %zu!", i + 1);
                    ui::label(buffer);
                }
                ui::container_end();
            }
            ui::scrollview_end();
        }
        ui::container_end();
        ui::end();
        // draw
        BeginDrawing();
        ClearBackground(RAYWHITE);
        ui::draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
