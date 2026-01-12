#ifndef CHECKBOX_NODE_H
#define CHECKBOX_NODE_H

#include <raylib.h>

#include "draw.h"

typedef void (*CheckboxNodeOnClickCallback)(bool value);

typedef struct CheckboxNode
{
    Rectangle rect;
    Vector2 position;
    Vector2 *screen;
    Side side;
    CheckboxNodeOnClickCallback on_click;
    bool state;
} CheckboxNode;

CheckboxNode checkbox_node_init(Vector2 *const screen, const Side side, CheckboxNodeOnClickCallback on_click);
Vector2 checkbox_node_update(CheckboxNode *const self, const Vector2 position, const Side side);
void checkbox_node_draw(const CheckboxNode *const self);

#endif /* end of include guard: CHECKBOX_NODE_H */
