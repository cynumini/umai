#pragma once

#include <raylib.h>

#include "sakana.hpp"

using namespace sakana;

namespace ui
{
namespace style
{
extern f32 padding;
extern f32 child_gap;
extern i32 font_size;
} // namespace style

struct Size
{
    enum SizeType
    {
        fit,
        grow,
        fixed
    } type = fit;
    f32 value = 0;
};

Size size_fit();
Size size_grow();
Size size_fixed(f32 value);

struct Sides
{
    f32 top = style::padding, left = style::padding, bottom = style::padding, right = style::padding;
};

Sides sides_all(f32 value);

enum class Direction
{
    left_to_right,
    top_to_bottom,
};

enum class Alignment
{
    left_top,
    left_center,
    left_bottom,
    center_top,
    center,
    center_bottom,
    right_top,
    right_center,
    right_bottom
};

struct RootContainerOptions
{
    Color background = BLANK;
    Alignment alignment = Alignment::left_top;
    Direction direction = Direction::left_to_right;
    Sides padding = {};
};

struct ContainerOptions
{
    Color background = BLANK;
    Size width = {};
    Size height = {};
    Sides padding = {};
    f32 border_size = 0;
    Alignment alignment = Alignment::left_top;
    Direction direction = Direction::left_to_right;
};
void begin(f32 width, f32 height, RootContainerOptions options = {});
void end();
void container_begin(ContainerOptions options = {});
void container_end();

struct ScrollViewOptions
{
    Color background = BLANK;
    Size width = {};
    Size height = {};
    Sides padding = {};
    f32 border_size = 0;
};
void scrollview_begin(f32 *scroll, ScrollViewOptions options = {});
void scrollview_end();

Rectangle calc_button_rectangle(const char *text, Vector2 offset = {0, 0});
bool button(const char *text);
void input(char *buffer, f32 width, const char *placeholder = "");
struct LabelOptions
{
    Sides padding = {};
};
void label(const char *text, LabelOptions options = {});
void row_begin();
void row_end();
void same_line();
void tab_begin(const char *name);
void tab_container_begin(usize *active_tab_id);
void tab_end();
void table_begin();
void table_end();
void temporarily_change_offset(Vector2 offset);

void draw();
} // namespace ui
