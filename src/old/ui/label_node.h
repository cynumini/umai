#ifndef LABEL_NODE_H
#define LABEL_NODE_H

#include <raylib.h>

#include "draw.h"

typedef struct LabelNode
{
    const char *text;
    Rectangle rect;
    Vector2 position;
    Vector2 *screen;
    Side side;
} LabelNode;

LabelNode label_node_init(const char *text, Vector2 *const screen, const Side side);
Vector2 label_node_update(LabelNode *const self, const Vector2 position, const Side side);
void label_node_draw(const LabelNode *const self);

#endif // LABEL_NODE_H
