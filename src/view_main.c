#include "view_main.h"
#include <raymath.h>

void view_main_init(ViewMain *self, Vector2 *screen, sqlite3 *database)
{
    self->screen = screen;
    self->database = database;

    self->label_title = label_node_init("Main", screen, SIDE_TOP_WIDE);
}

void view_main_deinit(ViewMain *self) {
    (void)self;
}

void view_main_update(ViewMain *self)
{
    Vector2 offset = Vector2Zero();
    offset = label_node_update(&self->label_title, offset, SIDE_BOTTOM_LEFT);
}

void view_main_draw(ViewMain const *self)
{
    DrawRectangle(0, 0, self->screen->x, 20, GRAY);
    label_node_draw(&self->label_title);
}
