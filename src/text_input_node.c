#include "text_input_node.h"

TextInputNode text_input_node_init(Vector2 *const screen, const Side side)
{
    return (TextInputNode){{0}, 0, {0, 0, 0, 0}, {0, 0}, screen, side};
}

void text_input_node_update(TextInputNode *const self, const Vector2 position)
{
    Result result = calc_rect(position, (Vector2){0, 24}, self->side, *self->screen);
    self->rect = result.rect;
    self->position = result.position;
    if (CheckCollisionPointRec(GetMousePosition(), self->rect))
    {
        int c;
        while ((c = GetCharPressed()))
        {
            self->text[self->text_len] = c;
            self->text_len++;
        }
        if (IsKeyPressed(KEY_BACKSPACE) && self->text_len)
        {
            self->text_len--;
            self->text[self->text_len] = 0;
        }
    }
}

void text_input_node_draw(const TextInputNode *const self) {
    DrawRectangleLinesEx(self->rect, 1, BLACK);
    DrawText(self->text, self->rect.x + 2, self->rect.y + 2, 20, BLACK);
}
