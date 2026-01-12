#include "table_node.h"

TableNode table_node_init(Vector2 *const screen, const Side side)
{
    return (TableNode){{0, 0, 0, 0}, {0, 0}, screen, side};
}

Vector2 table_node_update(TableNode *const self, const Vector2 position, const Side side)
{
    Result result = calc_rect(position, (Vector2){0, 20}, self->side, *self->screen);
    self->rect = result.rect;
    self->position = result.position;
    return calc_offset(self->rect, side);
}

void table_node_draw(const TableNode *const self)
{
    DrawRectangleLinesEx(self->rect, 1, BLACK);
}
