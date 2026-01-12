#ifndef TABLE_NODE_H
#define TABLE_NODE_H

#include <raylib.h>

#include "draw.h"

typedef struct TableNode
{
    Rectangle rect;
    Vector2 position;
    Vector2 *screen;
    Side side;
} TableNode;

TableNode table_node_init(Vector2 *const screen, const Side side);
Vector2 table_node_update(TableNode *const self, const Vector2 position, const Side side);
void table_node_draw(const TableNode *const self);

#endif /* end of include guard: TABLE_NODE_H */
