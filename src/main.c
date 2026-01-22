#include <stdlib.h>

#include <raylib.h>

#include "ui.h"

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    SetWindowState(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "umai");
    SetExitKey(0);
    SetTargetFPS(60);

    Context ctx = context_create();

    NODE(&ctx, {.color = RED})
    {
        NODE(&ctx, {.color = GREEN});
        NODE(&ctx, {.color = BLUE});
    }

    while (!WindowShouldClose())
    {
        // Update

        // Draw
        BeginDrawing();
        ClearBackground(WHITE);
        EndDrawing();
    }

    context_destroy(&ctx);

    CloseWindow();

    return EXIT_SUCCESS;
}
