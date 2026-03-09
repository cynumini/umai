#include "view_add_food.h"
#include "database.h"

#include <assert.h>
#include <raylib.h>
#include <raymath.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *image_path;

static bool kdialog(char *buffer, size_t buffer_size)
{
    char *command = "kdialog --getopenfilename ~/ \"Image Files (*.png *.jpg "
                    "*.jpeg *.jxl *.webp)\"";
    FILE *fpipe = popen(command, "r");
    assert(fpipe);
    bool result = false;
    if (fgets(buffer, buffer_size, fpipe))
    {
        size_t len = strlen(buffer);
        assert(len);
        buffer[len - 1] = 0;
        result = true;
    }
    pclose(fpipe);
    return result;
}

static void open_image(ImageNode *image_node)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        char line[FILENAME_MAX];
        if (kdialog(line, sizeof(line)))
        {
            image_path = line;
            UnloadTexture(image_node->texture);
            Image image = LoadImage(line);
            int new_height = 256;
            int new_width = (new_height * image.width) / image.height;
            ImageResize(&image, new_width, new_height);
            image_node->texture = LoadTextureFromImage(image);
            UnloadImage(image);
        }
    }
}

static void on_button_add_clicked(ButtonNode *const self, void *user_data)
{
    ViewAddFood *view = (ViewAddFood *)user_data;
    (void)self;
    printf("You entered: %s %s %s\n", view->text_input_name.text, view->text_input_energy.text, image_path);
    float energy = strtof(view->text_input_energy.text, NULL);
    database_add_food(view->database, view->text_input_name.text, energy);
}

void view_add_food_init(ViewAddFood *self, Vector2 *const screen, sqlite3 *database)
{
    self->texture = LoadTexture("assets/no_image.png");
    self->database = database;
    self->screen = screen;

    self->label_title = label_node_init("Add a new food", screen, SIDE_TOP_WIDE);
    self->label_image = label_node_init("Image", screen, SIDE_TOP_LEFT);
    self->image = image_node_init(self->texture, screen, SIDE_TOP_WIDE, &open_image);
    self->label_name = label_node_init("Name", screen, SIDE_TOP_LEFT);
    self->text_input_name = text_input_node_init(screen, SIDE_TOP_WIDE);
    self->label_energy = label_node_init("Energy", screen, SIDE_TOP_LEFT);
    self->text_input_energy = text_input_node_init(screen, SIDE_TOP_WIDE);

    self->button_cancel = button_node_init("Cancel", screen, SIDE_BOTTOM_RIGHT, NULL);
    self->button_add = button_node_init("Add", screen, SIDE_BOTTOM_RIGHT, on_button_add_clicked);
    self->button_add.user_data = self;
}

void view_add_food_deinit(ViewAddFood *const self)
{
    UnloadTexture(self->texture);
}

void view_add_food_update(ViewAddFood *const self)
{
    // left - top
    Vector2 offset = Vector2Zero();
    offset = label_node_update(&self->label_title, offset, SIDE_BOTTOM_LEFT);
    offset = label_node_update(&self->label_image, offset, SIDE_BOTTOM_LEFT);
    offset = image_node_update(&self->image, offset, SIDE_BOTTOM_LEFT);
    offset = label_node_update(&self->label_name, offset, SIDE_BOTTOM_LEFT);
    offset = text_input_node_update(&self->text_input_name, offset, SIDE_BOTTOM_LEFT);
    offset = label_node_update(&self->label_energy, offset, SIDE_BOTTOM_LEFT);
    offset = text_input_node_update(&self->text_input_energy, offset, SIDE_BOTTOM_LEFT);
    // right - bottom
    offset = *self->screen;
    offset = button_node_update(&self->button_cancel, offset, SIDE_BOTTOM_LEFT);
    offset = button_node_update(&self->button_add, offset, SIDE_BOTTOM_LEFT);
}

void view_add_food_draw(ViewAddFood const *const self)
{
    DrawRectangle(0, 0, self->screen->x, 20, GRAY);
    label_node_draw(&self->label_title);
    label_node_draw(&self->label_image);
    image_node_draw(&self->image);
    label_node_draw(&self->label_name);
    text_input_node_draw(&self->text_input_name);
    label_node_draw(&self->label_energy);
    text_input_node_draw(&self->text_input_energy);
    button_node_draw(&self->button_cancel);
    button_node_draw(&self->button_add);
}
