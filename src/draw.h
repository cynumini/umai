#ifndef DRAW_H
#define DRAW_H

#include <raylib.h>

typedef enum Side
{
    SIDE_TOP_LEFT,
    SIDE_TOP_WIDE,
    SIDE_BOTTOM_LEFT,
    SIDE_BOTTOM_RIGHT
} Side;

typedef struct Result
{
    Rectangle rect;
    Vector2 position;
} Result;

Result calc_rect(const Vector2 position, const Vector2 size, const Side side, const Vector2 screen);
Vector2 calc_offset(const Rectangle rect, const Side side);
Rectangle draw_label(const char *text, const Vector2 position, const Side side, const Vector2 screen);
Rectangle draw_image(const Texture2D texture, const Vector2 position, const Side side, const Vector2 screen);
Rectangle draw_text_input(const Vector2 position, const Side side, const Vector2 screen);
Rectangle draw_checkbox(const Vector2 position, const Side side, const Vector2 screen);

#endif // DRAW_H
