#include "checkbox_node.h"

CheckboxNode checkbox_node_init(Vector2 *const screen, const Side side, CheckboxNodeOnClickCallback on_click)
{
    return (CheckboxNode){{0, 0, 0, 0}, {0, 0}, screen, side, on_click, false};
}

Vector2 checkbox_node_update(CheckboxNode *const self, const Vector2 position, const Side side)
{
    Result result = calc_rect(position, (Vector2){20, 20}, self->side, *self->screen);
    self->rect = result.rect;
    self->position = result.position;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), self->rect))
    {
        self->state = !self->state;
        if (self->on_click)
        {
            self->on_click(self->state);
        }
    }
    return calc_offset(self->rect, side);
}

void checkbox_node_draw(const CheckboxNode *const self)
{
    DrawRectangleLinesEx(self->rect, 1, BLACK);
    if (self->state)
    {
        Vector2 top_left = {self->rect.x, self->rect.y};
        Vector2 top_right = {self->rect.x + self->rect.width, self->rect.y};
        Vector2 bottom_left = {self->rect.x, self->rect.y + self->rect.height};
        Vector2 bottom_right = {self->rect.x + self->rect.width, self->rect.y + self->rect.height};
        DrawLine(top_left.x, top_left.y, bottom_right.x, bottom_right.y, BLACK);
        DrawLine(top_right.x, top_right.y, bottom_left.x, bottom_left.y, BLACK);
    }
}
