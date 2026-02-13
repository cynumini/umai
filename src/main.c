#include <math.h>
#include <raylib.h>

#include "ui.h"

#include <SKN/arena.h>
#include <stdlib.h>

const char *long_text =
    "Lorem ipsum dolor sit amet consectetur adipiscing elit. Quisque faucibus ex sapien vitae pellentesque sem "
    "placerat. In id cursus mi pretium tellus duis convallis. Tempus leo eu aenean sed diam urna tempor. Pulvinar "
    "vivamus fringilla lacus nec metus bibendum egestas. Iaculis massa nisl malesuada lacinia integer nunc posuere. Ut "
    "hendrerit semper vel class aptent taciti sociosqu. Ad litora torquent per conubia nostra inceptos himenaeos.";

void test(const char *id)
{
    CONTRAINER({.id = id,
                .background = GREEN,
                .paddings = sides_all(8),
                .child_gap = 8,
                .height = size_grow(),
                .direction = DIRECTION_TOP_TO_BOTTOM})
    {
        CONTRAINER({.background = BLUE, .width = size_fixed(200), .height = size_fixed(200)});
        CONTRAINER({.background = GOLD, .width = size_fixed(100), .height = size_fixed(100)});
    }
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    SetWindowState(FLAG_WINDOW_RESIZABLE);

    const u32 width = 1380;
    const u32 height = 720;

    InitWindow(width, height, "umai");

    // SetExitKey(0);
    SetTargetFPS(60);

    ui_init();

    CONTRAINER({.id = "root",
                .background = RED,
                .paddings = sides_all(10),
                .child_gap = 8,
                .width = size_fixed(width),
                .height = size_fixed(height)})
    {
        TEXT({.id = "1", .text = long_text, .wrap = true, .size = 20});
        TEXT({.id = "2", .text = "A very long text, actually", .wrap = true, .size = 20});
        CONTRAINER({.width = size_fixed(256), .height = size_fixed(256), .background = GREEN});
        TEXT({.id = "3", .text = "A", .wrap = false, .size = 20});
        CONTRAINER({.width = size_fixed(128), .height = size_fixed(128), .background = BLUE});
        CONTRAINER({.width = size_fixed(128), .height = size_grow(), .background = YELLOW});
        test("here");
    }

    ui_print_tree();

    while (!WindowShouldClose())
    {
        // Update
        if (IsWindowResized())
        {
            ui_resize(GetScreenWidth(), GetScreenHeight());
        }

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
