#include <stdio.h>
#include <assert.h>

#include <raylib.h>

#include "sakana.h"
#include "ui.h"
#include "optional.h"


i32 main()
{
    u32 width = 1280, height = 720;
    i32 scroll = 0;
    // SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow((i32)width, (i32)height, "umai");
    RenderTexture v = LoadRenderTexture(0, 0);

    while (!WindowShouldClose())
    {
        // update
        if (IsWindowResized())
        {
            width = (u32)GetScreenWidth();
            height = (u32)GetScreenHeight();
        }
        ui_begin((ContainerOptions){
            .background = color_optional_create(RED),
            .width = size_fixed(width),
            .height = size_fixed(height),
        });
        {
            ui_begin((ContainerOptions){
                .background = color_optional_create(GREEN),
                .width = size_grow(),
                .height = size_grow(),
            });
            {
                ui_label("Yes, I am!", (TextOptions){});
                ui_begin((ContainerOptions){
                    .background = color_optional_create(BLUE),
                    .width = size_fixed(100),
                    .height = size_fixed(100),
                });
                ui_end();
                ui_scroll_view_begin(&scroll, &v,
                                     (ScrollViewOptions){
                                         .width = size_grow(),
                                         .height = size_grow(),
                                     });
                {
                    ui_begin((ContainerOptions){.direction = DIRECTION_TOP_TO_BOTTOM});
                    for (usize i = 0; i < 500; i++)
                    {
                        static char buffer[TEXT_MAX_LEN];
                        i32 r = snprintf(buffer, TEXT_MAX_LEN, "Hello, number %zu!", i + 1);
                        assert(r < TEXT_MAX_LEN);
                        ui_label(buffer, (TextOptions){});
                    }
                    ui_end();
                }
                ui_scroll_view_end();
            }
            ui_end();
        }
        ui_end();
        // draw
        BeginDrawing();
        ClearBackground(WHITE);
        ui_draw();
        EndDrawing();
    }
    UnloadRenderTexture(v);
    CloseWindow();
    return 0;
}
