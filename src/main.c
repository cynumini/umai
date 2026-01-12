#include "database.h"
#include "view_add_food.h"
#include "view_main.h"

typedef enum View
{
    VIEW_ADD_FOOD,
    VIEW_MAIN
} View;

int main(void)
{
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    Vector2 screen = {1280, 720};
    InitWindow(screen.x, screen.y, "umai");
    sqlite3 *database = database_init();

    View current_view = VIEW_MAIN;

    ViewAddFood view_add_food;
    view_add_food_init(&view_add_food, &screen, database);
    ViewMain view_main;
    view_main_init(&view_main, &screen, database);

    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        // Update
        if (IsWindowResized())
        {
            screen.x = GetScreenWidth();
            screen.y = GetScreenHeight();
        }
        switch (current_view)
        {
        case VIEW_MAIN:
            view_main_update(&view_main);
            break;
        case VIEW_ADD_FOOD:
            view_add_food_update(&view_add_food);
            break;
        }
        // Draw
        BeginDrawing();
        ClearBackground(WHITE);
        switch (current_view)
        {
        case VIEW_MAIN:
            view_main_draw(&view_main);
            break;
        case VIEW_ADD_FOOD:
            view_add_food_draw(&view_add_food);
            break;
        }
        EndDrawing();
    }
    database_deinit(database);
    view_add_food_deinit(&view_add_food);
    view_main_deinit(&view_main);
    CloseWindow();
    return 0;
}
