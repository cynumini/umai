#ifndef BUTTON_NODE_H
#define BUTTON_NODE_H

#include <raylib.h>

#include "draw.h"

typedef struct ButtonNode ButtonNode;

typedef void (*ButtonNodeOnClickCallback)(ButtonNode *const, void *user_data);

struct ButtonNode
{
    const char *text;
    Rectangle rect;
    Vector2 position;
    Vector2 *screen;
    Side side;
    ButtonNodeOnClickCallback on_click;
    void *user_data;
};

ButtonNode button_node_init(const char *text, Vector2 *const screen, const Side side,
                            ButtonNodeOnClickCallback on_click);
Vector2 button_node_update(ButtonNode *const self, const Vector2 position, const Side side);
void button_node_draw(const ButtonNode *const self);

#endif /* end of include guard: BUTTON_NODE_H */
