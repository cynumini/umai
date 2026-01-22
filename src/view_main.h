#ifndef VIEW_MAIN_H
#define VIEW_MAIN_H

#include <raylib.h>
#include <sqlite3.h>

#include "database.h"
#include "old-ui.h"

typedef struct ViewMain
{
    // Data
    sqlite3 *database;
    Foods foods;
    // UI
    Size width;
    Size height;
    OldElement root;
    OldElement element_header;
    Text text_header;
    OldElement main;
    Table table;
    OldElement sidebar;
    Text name;
    TextInput text_input_name;
} ViewMain;

void view_main_init(ViewMain *self, sqlite3 *database);
void view_main_deinit(ViewMain *self);
void view_main_calc_layout(ViewMain *self);
void view_main_update(ViewMain *self);
void view_main_draw(ViewMain *self);

#endif /* end of include guard: VIEW_MAIN_H */
