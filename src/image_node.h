#ifndef IMAGE_NODE_H
#define IMAGE_NODE_H

#include <raylib.h>

#include "draw.h"

typedef struct ImageNode ImageNode;

typedef void (*ImageNodeOnClickCallback)(ImageNode *);

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
void image_node_update(ImageNode *const self, const Vector2 position);
void image_node_draw(const ImageNode *const self);

#endif // !IMAGE_NODE_H
