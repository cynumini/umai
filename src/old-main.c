#include "ui.h"
#include <raylib.h>
#include <stddef.h>
#include <stdlib.h>

int old_main(void)
{
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "umai");
    SetTargetFPS(60);

    Element root = element_init();
    root.node.width = size_fixed(1280);
    root.node.height = size_fixed(720);
    root.padding = padding_all(32);
    root.child_gap = 32;
    root.node.color = BLUE;
    Element a = element_init();
    a.layout_direction = LAYOUT_DIRECTION_TOP_TO_BOTTOM;
    a.child_gap = 16;
    a.padding = padding_all(16);
    a.node.width = size_grow();
    a.node.height = size_grow();
    a.node.color = PINK;
    Element a_a = element_init();
    a_a.node.width = size_fixed(100);
    a_a.node.height = size_fixed(100);
    a_a.node.color = RED;
    Element a_b = element_init();
    a_b.node.width = size_fixed(100);
    a_b.node.height = size_fixed(75);
    a_b.node.color = GREEN;
    Text a_c = text_init(
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam sit amet pretium ligula, at gravida quam. "
        "Integer iaculis, velit at sagittis ultricies, lacus metus scelerisque turpis, ornare feugiat nulla nisl ac "
        "erat. Maecenas elementum ultricies libero, sed efficitur lacus molestie non. Nulla ac pretium dolor. "
        "Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Orci varius "
        "natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Pellentesque mi lorem, consectetur "
        "id porttitor id, sollicitudin sit amet enim. Duis eu dolor magna. Nunc ut augue turpis.",
        true);
    Element b = element_init();
    b.node.width = size_grow();
    b.node.height = size_grow();
    b.node.color = YELLOW;

    element_add_child(&root, &a.node);
    element_add_child(&root, &b.node);
    element_add_child(&a, &a_a.node);
    element_add_child(&a, &a_b.node);
    element_add_child(&a, &a_c.node);

    root.node.calc_layout(&root.node);
    while (!WindowShouldClose())
    {
        if (IsWindowResized())
        {
            root.node.width = size_fixed(GetScreenWidth());
            root.node.height = size_fixed(GetScreenHeight());
            root.node.calc_layout(&root.node);
        }
        root.node.update(&root.node);
        BeginDrawing();
        ClearBackground(WHITE);
        root.node.draw(&root.node);
        EndDrawing();
    }
    CloseWindow();

    element_deinit(&root);
    element_deinit(&a);
    text_deinit(&a_c);

    return EXIT_SUCCESS;
}
