#include <cstdio>
#include <cstdlib>

#include <SKN/types.hpp>
#include <raylib.h>

#include "SKN/arena.hpp"
#include "SKN/array.hpp"
#include "ui.hpp"

struct Window
{
    Window(i32 width, i32 height, const char *title)
    {
        InitWindow(width, height, title);
    }

    ~Window()
    {
        CloseWindow();
    }
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    Window window = Window{1280, 720, "umai"};
    SetTargetFPS(60);

    {
        auto arena = sakana::Arena(MB(1));

        auto array = sakana::Array<int>();

        array.add(&arena, 5);

        // printf("my value is = %i\n", *value);
        // *value = 10;
        // printf("my value is = %i\n", *value);
    }

    while (!WindowShouldClose())
    {
        BeginDrawing();
        {
            ui_begin();
            if (button("My new dear button"))
            {
                printf("nya 1\n");
            }
            if (button("My dear button"))
            {
                printf("nya 2\n");
            }
            ClearBackground(WHITE);
        }
        EndDrawing();
    }
    printf("umai\n");

    return EXIT_SUCCESS;
}
