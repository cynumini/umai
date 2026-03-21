#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <raylib.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;
static_assert(sizeof(f32) == 4);
static_assert(sizeof(f64) == 8);

typedef size_t usize;
typedef intptr_t isize;

typedef struct
{
    u32 top;
    u32 left;
    u32 bottom;
    u32 right;
} Paddings;

typedef struct
{
    Paddings paddings;
    Color background;
    Color foreground;
} Style;

static Style current_style = {
    .paddings = {8, 8, 8, 8},
    .background = {0},
    .foreground = {0},
};

typedef enum
{
    DIRECTION_LEFT_TO_RIGHT,
    DIRECTION_TOP_TO_BOTTOM,
} Direction;

typedef struct
{
    Paddings paddings;
    Color background;
    Color foreground;
    usize parent_index;
    Direction direction;
} Node;

typedef enum
{
    SIZE_TYPE_FIT,
    SIZE_TYPE_GROW,
    SIZE_TYPE_FIXED
} SizeType;

typedef struct
{
    SizeType type;
    u32 size;
} Size;

// static Size size_fit() { return (Size){0}; }
// static Size size_grow() { return (Size){.type = SIZE_TYPE_GROW}; }
static Size size_fixed(u32 size) { return (Size){.type = SIZE_TYPE_FIXED, .size = size}; }

typedef struct
{
    Direction direction;
    Size width;
    Size height;
} ContainerOptions;

#define NODES_MAX_LEN 256
static Node nodes[NODES_MAX_LEN];
static usize current_container_index = 0;
static usize current_index = 0;

static usize add_node(Node node)
{
    assert((current_index + 1) < NODES_MAX_LEN);
    nodes[current_index] = node;
    nodes[current_index].parent_index = current_container_index;
    return current_index++;
}

static void ui_begin(ContainerOptions options)
{
    current_container_index = add_node((Node){
        .paddings = current_style.paddings,
        .direction = options.direction,
    });
}

static void ui_end()
{
    usize parent_index = nodes[current_container_index].parent_index;
    // current_container_index == parent_index means it's last ui_end
    if (current_container_index == parent_index)
    {
        current_index = 0;
    }
    else
    {
        current_container_index = parent_index;
    }
}

i32 main()
{
    u32 width = 1280, height = 720;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow((i32)width, (i32)height, "umai");

    while (!WindowShouldClose())
    {
        // update
        if (IsWindowResized())
        {
            width = (u32)GetScreenWidth();
            height = (u32)GetScreenHeight();
        }
        ui_begin((ContainerOptions){.width = size_fixed(width), .height = size_fixed(height)});
        {
            ui_begin((ContainerOptions){});
            {
                ui_begin((ContainerOptions){});
                ui_end();
                ui_begin((ContainerOptions){});
                ui_end();
            }
            ui_end();
        }
        ui_end();
        // draw
        BeginDrawing();
        ClearBackground(WHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
