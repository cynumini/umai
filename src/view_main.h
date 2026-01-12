#ifndef VIEW_MAIN_H
#define VIEW_MAIN_H

#include "ui/label_node.h"
#include <raylib.h>
#include <sqlite3.h>

typedef struct ViewMain
{
    sqlite3 *database;
    Vector2 *screen;
    LabelNode label_title;
} ViewMain;

void view_main_init(ViewMain *self, Vector2 *screen, sqlite3 *database);
void view_main_deinit(ViewMain *self);
void view_main_update(ViewMain *self);
void view_main_draw(ViewMain const *self);

#endif /* end of include guard: VIEW_MAIN_H */
