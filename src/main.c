#include <math.h>
#include <raylib.h>

#include "ui.h"

#include <SKN/arena.h>
#include <stdio.h>
#include <stdlib.h>

const char *long_text =
    "Lorem ipsum dolor sit amet consectetur adipiscing elit. Quisque faucibus ex sapien vitae pellentesque sem "
    "placerat. In id cursus mi pretium tellus duis convallis. Tempus leo eu aenean sed diam urna tempor. Pulvinar "
    "vivamus fringilla lacus nec metus bibendum egestas. Iaculis massa nisl malesuada lacinia integer nunc posuere. Ut "
    "hendrerit semper vel class aptent taciti sociosqu. Ad litora torquent per conubia nostra inceptos himenaeos.";

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    SetWindowState(FLAG_WINDOW_RESIZABLE);

    const u32 width = 1175;
    const u32 height = 720;

    InitWindow(width, height, "umai");

    // SetExitKey(0);
    SetTargetFPS(60);

    ui_init();
    TABS({.id = "root"})
    {
        TEXT({.id = "Main", .text = "Main", .size = 20});
        TEXT({.id = "Add a food", .text = "Work", .size = 20});
    }

    ui_print_tree();

    while (!WindowShouldClose())
    {
        // Update
        if (IsKeyReleased(KEY_SPACE))
        {
            Tabs *root = (Tabs *)ui_get_by_id("root");
            root->current = root->current == 0 ? 1 : 0;
            ui_commit();
        }

        ui_update();

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
