#include <raylib.h>

#include "ui.h"

#include <SKN/arena.h>
#include <stdlib.h>

void test(const char *id)
{
    CONTRAINER({.id = id, .background = GREEN, .paddings = sides_all(8), .child_gap = 8, .height = size_grow()})
    {
        CONTRAINER({.background = BLUE, .width = size_fixed(200), .height = size_fixed(200)});
        CONTRAINER({.background = GOLD, .width = size_fixed(100), .height = size_grow()});
    }
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    SetWindowState(FLAG_WINDOW_RESIZABLE);

    InitWindow(1280, 720, "umai");

    // SetExitKey(0);
    SetTargetFPS(60);

    ui_init();

    CONTRAINER({.background = RED, .paddings = sides_all(10), .child_gap = 8})
    {
        CONTRAINER({.width = size_fixed(256), .height = size_fixed(256), .background = GREEN});
        CONTRAINER({.width = size_fixed(128), .height = size_fixed(128), .background = BLUE});
        CONTRAINER({.width = size_fixed(128), .height = size_grow(), .background = YELLOW});
        test("test");
    }

    ui_print_tree();

    while (!WindowShouldClose())
    {
        // Update

        // Draw
        BeginDrawing();
        {
            ClearBackground(WHITE);
            ui_draw();
        }
        EndDrawing();
    }

    ui_deinit();
    CloseWindow();
    return EXIT_SUCCESS;
}
