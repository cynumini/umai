#include "draw.h"

Vector2 calc_offset(const Rectangle rect, const Side side)
{
    Vector2 offset = {rect.x, rect.y};
    switch (side)
    {
    case SIDE_TOP_LEFT:
        break;
    case SIDE_TOP_WIDE:
        offset.x = rect.x + rect.width / 2;
        break;
    case SIDE_BOTTOM_LEFT:
        offset.y = rect.y + rect.height;
        break;
    case SIDE_BOTTOM_RIGHT:
        offset.x = rect.x + rect.width;
        offset.y = rect.y + rect.height;
        break;
    }
    return offset;
}

Result calc_rect(const Vector2 position, const Vector2 size, const Side side, const Vector2 screen)
{
    Result result = {{position.x, position.y, size.x, size.y}, position};
    switch (side)
    {
    case SIDE_TOP_WIDE:
        result.position.x = (screen.x - result.rect.width) / 2;
        result.rect.width = screen.x - result.rect.x;
        break;
    case SIDE_TOP_LEFT:
        break;
    case SIDE_BOTTOM_LEFT:
        result.position.y -= result.rect.height;
        result.rect.y = result.position.y;
        break;
    case SIDE_BOTTOM_RIGHT:
        result.position.x -= result.rect.width;
        result.rect.x = result.position.x;
        result.position.y -= result.rect.height;
        result.rect.y = result.position.y;
        break;
    }
    return result;
}

Rectangle draw_label(const char *text, const Vector2 position, const Side side, const Vector2 screen)
{
    Result result = calc_rect(position, (Vector2){MeasureText(text, 20), 20}, side, screen);
    DrawText(text, result.position.x, result.position.y, result.rect.height, BLACK);
    return result.rect;
}

Rectangle draw_text_input(const Vector2 position, const Side side, const Vector2 screen)
{
    Result result = calc_rect(position, (Vector2){0, 20}, side, screen);
    DrawRectangleLinesEx(result.rect, 1, BLACK);
    return result.rect;
}

Rectangle draw_checkbox(const Vector2 position, const Side side, const Vector2 screen)
{
    Result result = calc_rect(position, (Vector2){20, 20}, side, screen);
    DrawRectangleLinesEx(result.rect, 1, BLACK);
    return result.rect;
}
