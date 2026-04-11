#include <raylib.h>
#include <stddef.h>

typedef int i32;
typedef float f32;
typedef size_t usize;
static_assert(sizeof(i32) == 4);
static_assert(sizeof(f32) == 4);

typedef struct {
  Color clear_color;
  Color foreground;
  Color background;
  Color hover;
  Color active;
  f32 padding;
  i32 font_size;
  f32 border;
} Style;

Style style;

Style style_init() {
  return (Style){.clear_color = LIGHTGRAY,
                 .foreground = BLACK,
                 .background = GRAY,
                 .hover = DARKGRAY,
                 .active = LIME,
                 .padding = 6,
                 .font_size = 20,
                 .border = 2};
}

Rectangle ui_rec = {};
Vector2 ui_offset = {};
Vector2 ui_mouse_position = {};
usize ui_current_index = 0;
bool ui_mouse_pressed = false;
bool ui_mouse_down = false;

void ui_frameStart() {
  ui_mouse_position = GetMousePosition();
  ui_mouse_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
  ui_mouse_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
}

void frameEnd() {}

void ui_start(Vector2 position) { ui_offset = position; }

usize getIndex() {
  auto index = ui_current_index;
  ui_current_index++;
  return index;
}

bool ui_button(const char *text) {
  Rectangle rec = {
      .x = ui_offset.x,
      .y = ui_offset.y,
      .width = (f32)MeasureText(text, style.font_size) + style.padding * 2,
      .height = (f32)style.font_size + style.padding * 2,
  };
  bool click = false;
  Color background = style.background;
  if (CheckCollisionPointRec(ui_mouse_position, rec)) {
    if (ui_mouse_pressed) {
      click = true;
    } else if (ui_mouse_down) {
      background = style.hover;
    } else {
      background = style.active;
    }
  }
  DrawRectangleRec(rec, background);
  DrawRectangleLinesEx(rec, style.border, style.foreground);
  DrawText(text, (i32)(ui_offset.x + style.padding),
           (i32)(ui_offset.y + style.padding), style.font_size,
           style.foreground);
  return click;
}

typedef struct {
  i32 current_tab;
} State;

void uiTabs([[maybe_unused]] Rectangle rec, [[maybe_unused]] State *state) {
  ui_start((Vector2){rec.x, rec.y});
}

i32 main() {
  style = style_init();
  State state = {};
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  i32 width = 1280, height = 720;
  InitWindow(width, height, "umai");
  SetTargetFPS(60);
  SetExitKey(KEY_F1);
  while (!WindowShouldClose()) {
    ui_frameStart();
    if (IsWindowResized()) {
      width = GetScreenWidth();
      height = GetScreenHeight();
    }
    BeginDrawing();
    ClearBackground(style.clear_color);
    Rectangle bar_rec = {};
    {
      ui_start((Vector2){style.padding, style.padding});
      ui_button("add a food");
      bar_rec.height = ui_rec.height + style.padding * 2;
    }
    const f32 max_width = (f32)width / 6;
    uiTabs(
        (Rectangle){
            .x = 0, .y = bar_rec.height, .width = max_width, .height = 0},
        &state);
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
