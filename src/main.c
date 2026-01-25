#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <raylib.h>

#include "ui.h"

#include <SKN/arena.h>


int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    SetWindowState(FLAG_WINDOW_RESIZABLE);

    InitWindow(1280, 720, "umai");

    // SetExitKey(0);
    SetTargetFPS(60);

    Arena arena = arena_create(MB(1));
    Context ctx = context_create(&arena);
    (void)ctx;

    NODE(&ctx, {.id = "1", .color = RED})
    {
        NODE(&ctx, {.id = "1.1", .color = GREEN});
        NODE(&ctx, {.id = "1.2", .color = BLUE});
    }

    node_print(ctx.current, 0);

    while (!WindowShouldClose())
    {
        // Update

        // Draw
        BeginDrawing();
        ClearBackground(WHITE);
        EndDrawing();
    }
    //
    // // Do I need it, since arena destroy already destory all context data?
    // // context_destroy(&ctx);
    arena_destroy(&arena);

    CloseWindow();

    return EXIT_SUCCESS;
}
