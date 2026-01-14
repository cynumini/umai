#include "view_main.h"
#include <raymath.h>
#include <stdio.h>

void view_main_init(ViewMain *self, Vector2 *screen, sqlite3 *database)
{
    self->screen = screen;
    self->database = database;

    self->label_title = label_node_init("Main", screen, SIDE_TOP_WIDE);
    self->foods = database_select_food(database);
}

void view_main_deinit(ViewMain *self)
{
    database_deinit_food(&self->foods);
}

void view_main_update(ViewMain *self)
{
    Vector2 offset = Vector2Zero();
    offset = label_node_update(&self->label_title, offset, SIDE_BOTTOM_LEFT);
}

void view_main_draw(ViewMain const *self)
{
    int y = 20;
    char buffer[256];
    for (size_t i = 0; i < self->foods.len; i++)
    {
        Food *food = &self->foods.items[i];
        sprintf(buffer, "%li", food->id);
        DrawText(buffer, 0, y, 20, BLACK);

        sprintf(buffer, "%li", food->created);
        DrawText(buffer, 150, y, 20, BLACK);

        DrawText(food->name, 300, y, 20, BLACK);

        sprintf(buffer, "%f", food->energy);
        DrawText(buffer, 450, y, 20, BLACK);

        y += 20;
    }
    DrawRectangle(0, 0, self->screen->x, 20, GRAY);
    label_node_draw(&self->label_title);
}
