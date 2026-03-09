#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

#include "ui.hpp"

#include <SKN/types.hpp>
#include <raylib.h>

enum struct ButtonState : u8
{
    up,
    down,
    released,
};

static Vector2 mouse_position;
static ButtonState mouse_button_left;

void ui_begin()
{
    mouse_position = GetMousePosition();
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        mouse_button_left = ButtonState::released;
    }
    else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        mouse_button_left = ButtonState::down;
    }
    else
    {
        mouse_button_left = ButtonState::up;
    }
}

void row_begin();

bool button(const char *text)
{
    f32 width = MeasureText(text, 20) + 4;
    auto rect = Rectangle{0, 0, width, 20 + 4};

    bool clicked = false;
    auto background = GRAY;

    if (CheckCollisionPointRec(mouse_position, rect))
    {
        switch (mouse_button_left)
        {
        case ButtonState::up:
            background = BLUE;
            break;
        case ButtonState::released:
            clicked = true;
        case ButtonState::down:
            background = DARKGRAY;
            break;
        }
    }

    DrawRectangleRec(rect, background);
    DrawRectangleLinesEx(rect, 1, BLACK);
    DrawText(text, 2, 2, 20, BLACK);

    return clicked;
}
