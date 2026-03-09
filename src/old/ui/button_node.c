#include <stdlib.h>

#include "button_node.h"

ButtonNode button_node_init(const char *text, Vector2 *const screen, const Side side,
                            ButtonNodeOnClickCallback on_click)
{
    return (ButtonNode){text, {0, 0, 0, 0}, {0, 0}, screen, side, on_click, NULL};
}

Vector2 button_node_update(ButtonNode *self, const Vector2 position, const Side side)
{
    int text_height = 20;
    int text_width = MeasureText(self->text, text_height);
    float width = text_width + 4;
    float height = text_height + 4;
    Result result = calc_rect(position, (Vector2){width, height}, self->side, *self->screen);
    self->rect = result.rect;
    self->position = result.position;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), self->rect) &&
        self->on_click)
    {
        self->on_click(self, self->user_data);
    }
    return calc_offset(self->rect, side);
}

void button_node_draw(const ButtonNode *self)
{
    DrawRectangleLinesEx(self->rect, 1, BLACK);
    DrawText(self->text, self->position.x + 2, self->position.y + 2, 20, BLACK);
}




