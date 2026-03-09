#ifndef VIEW_ADD_FOOD_H
#define VIEW_ADD_FOOD_H

#include <raylib.h>
#include <sqlite3.h>

#include "ui/button_node.h"
#include "ui/image_node.h"
#include "ui/label_node.h"
#include "ui/text_input_node.h"

typedef struct ViewAddFood ViewAddFood;

struct ViewAddFood
{
    sqlite3 *database;
    Vector2 *screen;
    Texture texture;

    LabelNode label_title;
    LabelNode label_image;
    ImageNode image;
    LabelNode label_name;
    TextInputNode text_input_name;
    LabelNode label_energy;
    TextInputNode text_input_energy;

    ButtonNode button_cancel;
    ButtonNode button_add;
};

void view_add_food_init(ViewAddFood *self, Vector2 *screen, sqlite3 *database);
void view_add_food_deinit(ViewAddFood *self);
void view_add_food_update(ViewAddFood *self);
void view_add_food_draw(ViewAddFood const *self);

#endif /* end of include guard: VIEW_ADD_FOOD_H */
