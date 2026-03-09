#include <raylib.h>

static_assert(sizeof(int) == 4);
using i32 = int;
static_assert(sizeof(float) == 4);
using f32 = float;

struct Window
{
    Window()
    {
        InitWindow(1280, 720, "umai");
    }
    ~Window()
    {
        CloseWindow();
    }
};

namespace UI
{

f32 window_width;
f32 window_height;

Vector2 mouse_position;
Rectangle previous;
Vector2 offset;
bool same_line = false;

bool button(const char *text)
{
    bool result = false;
    Vector2 local_offset;
    if (same_line)
    {
        local_offset = {previous.x + previous.width, previous.y};
    }
    else
    {
        local_offset = {offset.x, previous.y + previous.height};
    }
    i32 width = MeasureText(text, 20);
    i32 height = 20;
    Rectangle rectangle = {local_offset.x, local_offset.y, (float)(width + 4), (float)(height + 4)};
    Color background = LIGHTGRAY;
    if (CheckCollisionPointRec(mouse_position, rectangle))
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            result = true;
        }

        background = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? DARKGRAY : GRAY;
    }
    DrawRectangleRec(rectangle, background);
    DrawRectangleLinesEx(rectangle, 1, BLACK);
    DrawText(text, local_offset.x + 2, local_offset.y + 2, height, BLACK);
    previous = rectangle;
    same_line = false;
    return result;
}

void begin(f32 width, f32 height)
{
    offset = {};
    previous = {};
    window_width = width;
    window_height = height;
    mouse_position = GetMousePosition();
}

void tab_container_begin()
{
    Rectangle rectangle = {offset.x, offset.y, window_width, 20};
    DrawRectangleRec(rectangle, GRAY);
}

void tab_begin(const char *name)
{
    DrawText(name, 0, 0, 20, BLACK);
}

} // namespace UI

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    Window window;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(WHITE);
        UI::begin(GetScreenWidth(), GetScreenHeight());
        UI::tab_container_begin();
        UI::tab_begin("Main");
        UI::tab_begin("Second");
        UI::tab_begin("Third");
        EndDrawing();
    }
    return 0;
}
