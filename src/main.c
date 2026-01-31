#include <raylib.h>

#include "ui.h"

#include <SKN/arena.h>
#include <stdlib.h>

// void test(Context *ctx, const char *id)
// {
//     NODE(ctx, {.id = id, .color = GREEN, .padding = padding_all(8), .child_gap = 8, .height = size_grow()})
//     {
//         NODE(ctx, {.color = BLUE, .width = size_fixed(200), .height = size_fixed(200)});
//         NODE(ctx, {.color = GOLD, .width = size_fixed(100), .height = size_grow()});
//     }
// }
//
int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    SetWindowState(FLAG_WINDOW_RESIZABLE);

    InitWindow(1280, 720, "umai");

    // SetExitKey(0);
    SetTargetFPS(60);

    ui_init();

    CONTRAINER({0})
    {
        CONTRAINER({0});
        CONTRAINER({0});
    }
    // Arena arena = arena_create(MB(1));
    // UI ui = context_create(&arena);

    //     NODE(&ctx, {
    //                    .color = RED,
    //                    .padding = padding_all(8),
    //                    .child_gap = 8,
    //                    .width = size_fixed(1280),
    //                    .height = size_fixed(720),
    //                })
    //     {
    //         test(&ctx, "a");
    //         test(&ctx, "b");
    //     }
    //
    //     node_print(ctx.current, 0);
    //     context_calc_layout(&ctx);

    while (!WindowShouldClose())
    {
        // Update

        // Draw
        BeginDrawing();
        {
            ClearBackground(WHITE);
            //             context_draw(&ctx);
        }
        EndDrawing();
    }

    ui_deinit();
    CloseWindow();
    return EXIT_SUCCESS;
}
