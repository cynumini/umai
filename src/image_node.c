#include "image_node.h"

ImageNode image_node_init(const Texture texture, Vector2 *const screen, const Side side,
                          ImageNodeOnClickCallback on_click)
{
    return (ImageNode){texture, {0, 0, 0, 0}, {0, 0}, screen, side, on_click};
}

void image_node_update(ImageNode *self, const Vector2 position)
{
    Vector2 size = {self->texture.width, self->texture.height};
    Result result = calc_rect(position, size, self->side, *self->screen);

    self->rect = result.rect;
    self->position = result.position;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), self->rect) &&
        self->on_click)
        self->on_click(self);
}

void image_node_draw(const ImageNode *const self)
{
    DrawTexture(self->texture, self->position.x, self->position.y, WHITE);
}
