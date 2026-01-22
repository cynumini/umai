#include "view_main.h"
#include "ui.h"

#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <string.h>

static size_t get_row(void *data)
{
    Foods *foods = (Foods *)data;
    return foods->len;
}

static size_t get_column(void *data)
{
    (void)data;
    return 4;
}

static void get_cell(void *data, size_t row, size_t column, char *buffer)
{
    Foods *foods = (Foods *)data;
    Food food = foods->items[row];
    switch (column)
    {
    case 0:
        sprintf(buffer, "%li", food.id);
        break;
    case 1:
        sprintf(buffer, "%li", food.created);
        break;
    case 2:
        strcpy(buffer, food.name);
        break;
    case 3:
        sprintf(buffer, "%f", food.energy);
        break;
    }
}

static char name_buffer[256] = "fuck";

void on_row_selected(Table *self, int index, void *user_data)
{
    Text *name = (Text *)user_data;
    self->data.get_cell(self->data.data, index, 2, name_buffer);
    name->text = name_buffer;
}

void view_main_init(ViewMain *self, sqlite3 *database)
{
    self->database = database;

    self->root = element_init();
    self->root.layout_direction = LAYOUT_DIRECTION_TOP_TO_BOTTOM;
    self->element_header = element_init();
    self->element_header.padding = padding_all(4);
    self->element_header.node.height = size_fit();
    self->element_header.node.width = size_grow();
    self->element_header.node.color = GRAY;
    self->element_header.node.alignment = ALIGNMENT_CENTER;
    self->text_header = text_init("Main", false);

    self->foods = database_select_food(database);

    self->main = element_init();
    self->main.node.height = size_grow();
    self->main.node.width = size_grow();

    self->name = text_init("Test", false);
    self->text_input_name = text_input_init();
    strcpy(self->text_input_name.text, "aaa");

    TableData data = {&self->foods, get_row, get_column, get_cell};
    self->table = table_init(data);
    self->table.user_data = &self->name;
    self->table.on_row_selected = on_row_selected;

    self->sidebar = element_init();
    self->sidebar.layout_direction = LAYOUT_DIRECTION_TOP_TO_BOTTOM;
    self->sidebar.node.height = size_grow();
    self->sidebar.node.width = size_grow();
    self->sidebar.node.color = GRAY;

    element_add_child(&self->root, &self->element_header.node);
    element_add_child(&self->element_header, &self->text_header.node);
    element_add_child(&self->root, &self->main.node);

    element_add_child(&self->main, &self->table.node);
    element_add_child(&self->main, &self->sidebar.node);

    element_add_child(&self->sidebar, &self->name.node);
    element_add_child(&self->sidebar, &self->text_input_name.node);
}

void view_main_deinit(ViewMain *self)
{
    element_deinit(&self->root);
    element_deinit(&self->element_header);
    text_deinit(&self->text_header);
    element_deinit(&self->main);
    element_deinit(&self->sidebar);
    database_deinit_food(&self->foods);
}

void view_main_calc_layout(ViewMain *self)
{
    bool is_first_calc = self->root.node.width.type == SIZE_TYPE_FIT;
    if (IsWindowResized() || is_first_calc)
    {
        self->root.node.width = size_fixed(GetScreenWidth());
        self->root.node.height = size_fixed(GetScreenHeight());
        self->root.node.calc_layout(&self->root.node);
    }
}

void view_main_update(ViewMain *self)
{
    self->root.node.update(&self->root.node);
}

void view_main_draw(ViewMain *self)
{
    self->root.node.draw(&self->root.node);
    //     DrawRectangle(0, 0, self->screen->x, 20, GRAY);
    //     label_node_draw(&self->label_title);
    //     table_node_draw(&self->table);
}
