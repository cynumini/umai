#ifndef TEXT_INPUT_NODE_H
#define TEXT_INPUT_NODE_H

#include <raylib.h>
#include <stddef.h>

#include "draw.h"

typedef struct TextInputNode
{
    char text[256];
	size_t text_len;
    Rectangle rect;
    Vector2 position;
    Vector2 *screen;
    Side side;
} TextInputNode;

TextInputNode text_input_node_init(Vector2 *const screen, const Side side);
Vector2 text_input_node_update(TextInputNode *const self, const Vector2 position, const Side side);
void text_input_node_draw(const TextInputNode *const self);

#endif /* end of include guard: TEXT_INPUT_NODE_H */
