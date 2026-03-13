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
Rectangle calc_button_rectangle(const char *text, Vector2 offset = {0, 0});
bool button(const char *text);
void begin(f32 width, f32 height);
void input(char *buffer, f32 width, const char *placeholder = "");
void label(const char *text);
void row_begin();
void row_end();
void same_line();
void tab_begin(const char *name);
void tab_container_begin(usize *active_tab_id);
void tab_end();
void table_begin();
void table_end();
void temporarily_change_offset(Vector2 offset);
} // namespace ui
