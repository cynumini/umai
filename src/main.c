#include <stdlib.h>

#include <raylib.h>

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
    InitWindow(1280, 720, "umai");
    sqlite3 *database = database_init();

    View current_view = VIEW_MAIN;

    ViewMain view_main;
    view_main_init(&view_main, database);
    // ViewAddFood view_add_food;
    // view_add_food_init(&view_add_food, &screen, database);

    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        // Update
        switch (current_view)
        {
        case VIEW_MAIN:
            view_main_calc_layout(&view_main);
            view_main_update(&view_main);
            break;
        case VIEW_ADD_FOOD:
            // view_add_food_update(&view_add_food);
            break;
        }
        if (IsKeyReleased(KEY_TAB))
        {
            if (current_view == VIEW_MAIN)
                current_view = VIEW_ADD_FOOD;
            else if (current_view == VIEW_ADD_FOOD)
                current_view = VIEW_MAIN;
            view_main.foods = database_select_food(database);
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
            // view_add_food_draw(&view_add_food);
            break;
        }
        EndDrawing();
    }
    database_deinit(database);
    // view_add_food_deinit(&view_add_food);
    view_main_deinit(&view_main);
    CloseWindow();
    return EXIT_SUCCESS;
}
