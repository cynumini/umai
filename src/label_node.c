#include "label_node.h"

LabelNode label_node_init(const char *text, Vector2 *const screen, const Side side)
{
    return (LabelNode){text, {0, 0, 0, 0}, {0, 0}, screen, side};
}

void label_node_update(LabelNode *const self, const Vector2 position)
{
    float height = 20;
    float width = MeasureText(self->text, height);
    Result result = calc_rect(position, (Vector2){width, height}, self->side, *self->screen);
    self->rect = result.rect;
    self->position = result.position;
}

void label_node_draw(const LabelNode *const self)
{
    DrawText(self->text, self->position.x, self->position.y, self->rect.height, BLACK);
}
