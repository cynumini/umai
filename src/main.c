#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <raylib.h>
#include <raymath.h>

#include "button_node.h"
#include "draw.h"
#include "image_node.h"
#include "label_node.h"
#include "text_input_node.h"

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

void open_image(ImageNode *image_node)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        char line[FILENAME_MAX];
        if (kdialog(line, sizeof(line)))
        {
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

int main(void)
{
    Vector2 screen = {1280, 720};

    SetWindowState(FLAG_WINDOW_RESIZABLE);
    InitWindow(screen.x, screen.y, "umai");

    Texture texture = LoadTexture("assets/no_image.png");

    SetTargetFPS(60);

    LabelNode label_basic = label_node_init("Basic", &screen, SIDE_TOP_WIDE);
    LabelNode label_image = label_node_init("Image", &screen, SIDE_TOP_LEFT);
    ImageNode image = image_node_init(texture, &screen, SIDE_TOP_WIDE, &open_image);
    LabelNode label_name = label_node_init("Name", &screen, SIDE_TOP_LEFT);
    TextInputNode text_input_name = text_input_node_init(&screen, SIDE_TOP_WIDE);
    LabelNode label_energy = label_node_init("Energy", &screen, SIDE_TOP_LEFT);
    TextInputNode text_input_energy= text_input_node_init(&screen, SIDE_TOP_WIDE);

    ButtonNode button_cancel = button_node_init("Cancel", &screen, SIDE_BOTTOM_RIGHT, NULL);
    ButtonNode button_add = button_node_init("Add", &screen, SIDE_BOTTOM_RIGHT, NULL);

    while (!WindowShouldClose())
    {
        // Update
        if (IsWindowResized())
        {
            screen.x = GetScreenWidth();
            screen.y = GetScreenHeight();
        }

        Vector2 offset = screen;
        button_node_update(&button_cancel, offset);
        offset = calc_offset(button_cancel.rect, SIDE_BOTTOM_LEFT);
        button_node_update(&button_add, offset);

        offset = Vector2Zero();

        label_node_update(&label_basic, offset);
        offset = calc_offset(label_basic.rect, SIDE_BOTTOM_LEFT);
        label_node_update(&label_image, offset);
        offset = calc_offset(label_image.rect, SIDE_BOTTOM_LEFT);
        image_node_update(&image, offset);
        offset = calc_offset(image.rect, SIDE_BOTTOM_LEFT);
        label_node_update(&label_name, offset);
        offset = calc_offset(label_name.rect, SIDE_BOTTOM_LEFT);
        text_input_node_update(&text_input_name, offset);
        offset = calc_offset(text_input_name.rect, SIDE_BOTTOM_LEFT);
        label_node_update(&label_energy, offset);
        offset = calc_offset(label_energy.rect, SIDE_BOTTOM_LEFT);
        text_input_node_update(&text_input_energy, offset);
        offset = calc_offset(text_input_energy.rect, SIDE_BOTTOM_LEFT);

        // Draw
        BeginDrawing();
        ClearBackground(WHITE);

        // Rectangle rect = draw_label("Energy", offset, SIDE_TOP_LEFT, screen);
        // offset = calc_offset(rect, SIDE_BOTTOM_LEFT);

        Rectangle rect = draw_label("Tags", offset, SIDE_TOP_LEFT, screen);
        offset = calc_offset(rect, SIDE_BOTTOM_LEFT);

        rect = draw_text_input(offset, SIDE_TOP_WIDE, screen);
        offset = calc_offset(rect, SIDE_BOTTOM_LEFT);

        rect = draw_label("Has ingredients", offset, SIDE_TOP_LEFT, screen);
        offset = calc_offset(rect, SIDE_BOTTOM_LEFT);

        rect = draw_checkbox(offset, SIDE_TOP_LEFT, screen);
        offset = calc_offset(rect, SIDE_BOTTOM_LEFT);

        rect = draw_label("Ingredients", offset, SIDE_TOP_WIDE, screen);
        offset = calc_offset(rect, SIDE_BOTTOM_LEFT);

        rect = draw_label("Recipe", offset, SIDE_TOP_LEFT, screen);
        offset = calc_offset(rect, SIDE_BOTTOM_LEFT);

        rect = draw_text_input(offset, SIDE_TOP_WIDE, screen);
        offset = calc_offset(rect, SIDE_BOTTOM_LEFT);

        rect = draw_label("Ingredients", offset, SIDE_TOP_LEFT, screen);
        offset = calc_offset(rect, SIDE_BOTTOM_LEFT);

        rect = draw_text_input(offset, SIDE_TOP_WIDE, screen);
        offset = calc_offset(rect, SIDE_BOTTOM_LEFT);

        rect = draw_text_input(offset, SIDE_TOP_WIDE, screen);
        offset = calc_offset(rect, SIDE_BOTTOM_LEFT);

        label_node_draw(&label_basic);
        label_node_draw(&label_image);
        image_node_draw(&image);
        label_node_draw(&label_name);
        text_input_node_draw(&text_input_name);
        label_node_draw(&label_energy);
        text_input_node_draw(&text_input_energy);

        button_node_draw(&button_cancel);
        button_node_draw(&button_add);

        // DrawFPS(0, 0);
        EndDrawing();
    }

    UnloadTexture(texture);

    CloseWindow();
    return 0;
}
