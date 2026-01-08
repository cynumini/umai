#ifndef BUTTON_NODE_H
#define BUTTON_NODE_H

#include <raylib.h>

#include "draw.h"

typedef void (*ButtonNodeOnClickCallback)();

typedef struct ButtonNode
{
    const char *text;
    Rectangle rect;
    Vector2 position;
    Vector2 *screen;
    Side side;
    ButtonNodeOnClickCallback on_click;
} ButtonNode;

ButtonNode button_node_init(const char *text, Vector2 *const screen, const Side side,
                            ButtonNodeOnClickCallback on_click);
void button_node_update(ButtonNode *const button, const Vector2 position);
void button_node_draw(const ButtonNode *const button);

#endif // BUTTON_NODE_H
