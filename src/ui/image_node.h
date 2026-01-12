#ifndef IMAGE_NODE_H
#define IMAGE_NODE_H

#include <raylib.h>

#include "draw.h"

typedef struct ImageNode ImageNode;

typedef void (*ImageNodeOnClickCallback)(ImageNode *const);

struct ImageNode
{
    Texture texture;
    Rectangle rect;
    Vector2 position;
    Vector2 *screen;
    Side side;
    ImageNodeOnClickCallback on_click;
};

ImageNode image_node_init(const Texture texture, Vector2 *const screen, const Side side,
                          ImageNodeOnClickCallback on_click);
Vector2 image_node_update(ImageNode *const self, const Vector2 position, const Side side);
void image_node_draw(const ImageNode *const self);

#endif // !IMAGE_NODE_H
