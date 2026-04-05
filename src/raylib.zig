const std = @import("std");

/// Vector2, 2 components
pub const Vector2 = extern struct {
    /// Vector x component
    x: f32 = 0,
    /// Vector y component
    y: f32 = 0,

    pub fn toRectangle(self: Vector2, width: f32, height: f32) Rectangle {
        return .{
            .x = self.x,
            .y = self.y,
            .width = width,
            .height = height,
        };
    }

    pub fn toRectangleV(self: Vector2, sizes: Vector2) Rectangle {
        return self.toRectangle(sizes.x, sizes.y);
    }

    pub fn toRectangleZero(self: Vector2) Rectangle {
        return self.toRectangle(0, 0);
    }

    /// Add vector and float value
    pub fn addValue(self: Vector2, add: f32) Vector2 {
        return .{
            .x = self.x + add,
            .y = self.y + add,
        };
    }

    pub fn subtract(self: Vector2, other: Vector2) Vector2 {
        return .{
            .x = self.x - other.x,
            .y = self.y - other.y,
        };
    }

    extern fn CheckCollisionPointRec(point: Vector2, rec: Rectangle) bool;
    /// Check if point is inside rectangle
    pub fn checkCollisionRec(self: Vector2, rect: Rectangle) bool {
        return self.CheckCollisionPointRec(rect);
    }

    extern fn DrawLineV(startPos: Vector2, endPos: Vector2, color: Color) void;
    /// Draw a line (using gl lines)
    pub fn drawLine(self: Vector2, end_position: Vector2, color: Color) void {
        DrawLineV(self, end_position, color);
    }
};

/// Color, 4 components, R8G8B8A8 (32bit)
pub const Color = extern struct {
    /// Color red value
    r: u8 = 0,
    /// Color green value
    g: u8 = 0,
    /// Color blue value
    b: u8 = 0,
    /// Color alpha value
    a: u8 = 0,

    /// Light Gray
    pub const light_gray: Color = .{ .r = 200, .g = 200, .b = 200, .a = 255 };
    /// Gray
    pub const gray: Color = .{ .r = 130, .g = 130, .b = 130, .a = 255 };
    /// Dark Gray
    pub const dark_gray: Color = .{ .r = 80, .g = 80, .b = 80, .a = 255 };
    /// Yellow
    pub const yellow: Color = .{ .r = 253, .g = 249, .b = 0, .a = 255 };
    /// Gold
    pub const gold: Color = .{ .r = 255, .g = 203, .b = 0, .a = 255 };
    /// Orange
    pub const orange: Color = .{ .r = 255, .g = 161, .b = 0, .a = 255 };
    /// Pink
    pub const pink: Color = .{ .r = 255, .g = 109, .b = 194, .a = 255 };
    /// Red
    pub const red: Color = .{ .r = 230, .g = 41, .b = 55, .a = 255 };
    /// Maroon
    pub const maroon: Color = .{ .r = 190, .g = 33, .b = 55, .a = 255 };
    /// Green
    pub const green: Color = .{ .r = 0, .g = 228, .b = 48, .a = 255 };
    /// Lime
    pub const lime: Color = .{ .r = 0, .g = 158, .b = 47, .a = 255 };
    /// Dark Green
    pub const dark_green: Color = .{ .r = 0, .g = 117, .b = 44, .a = 255 };
    /// Sky Blue
    pub const skyblue: Color = .{ .r = 102, .g = 191, .b = 255, .a = 255 };
    /// Blue
    pub const blue: Color = .{ .r = 0, .g = 121, .b = 241, .a = 255 };
    /// Dark Blue
    pub const dark_blue: Color = .{ .r = 0, .g = 82, .b = 172, .a = 255 };
    /// Purple
    pub const purple: Color = .{ .r = 200, .g = 122, .b = 255, .a = 255 };
    /// Violet
    pub const violet: Color = .{ .r = 135, .g = 60, .b = 190, .a = 255 };
    /// Dark Purple
    pub const dark_purple: Color = .{ .r = 112, .g = 31, .b = 126, .a = 255 };
    /// Beige
    pub const beige: Color = .{ .r = 211, .g = 176, .b = 131, .a = 255 };
    /// Brown
    pub const brown: Color = .{ .r = 127, .g = 106, .b = 79, .a = 255 };
    /// Dark Brown
    pub const dark_brown: Color = .{ .r = 76, .g = 63, .b = 47, .a = 255 };

    /// White
    pub const white: Color = .{ .r = 255, .g = 255, .b = 255, .a = 255 };
    /// Black
    pub const black: Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };
    /// Blank (Transparent)
    pub const blank: Color = .{ .r = 0, .g = 0, .b = 0, .a = 0 };
    /// Magenta
    pub const magenta: Color = .{ .r = 255, .g = 0, .b = 255, .a = 255 };
    /// raysan5's White (raylib logo)
    pub const ray_white: Color = .{ .r = 245, .g = 245, .b = 245, .a = 255 };
};

extern fn ClearBackground(color: Color) void;
/// Set background color (framebuffer clear color)
pub fn clearBackground(color: Color) void {
    ClearBackground(color);
}

/// Rectangle, 4 components
pub const Rectangle = extern struct {
    /// Rectangle top-left corner position x
    x: f32 = 0,
    /// Rectangle top-left corner position y
    y: f32 = 0,
    /// Rectangle width
    width: f32 = 0,
    /// Rectangle height
    height: f32 = 0,

    pub fn toVector2(self: Rectangle) Vector2 {
        return .{ .x = self.x, .y = self.y };
    }

    extern fn DrawRectangleRec(rec: Rectangle, color: Color) void;
    /// Draw a color-filled rectangle
    pub fn draw(self: Rectangle, color: Color) void {
        self.DrawRectangleRec(color);
    }

    extern fn DrawRectangleLinesEx(rec: Rectangle, lineThick: f32, color: Color) void;
    /// Draw rectangle outline with extended parameters
    pub fn drawLines(self: Rectangle, line_thick: f32, color: Color) void {
        self.DrawRectangleLinesEx(line_thick, color);
    }
};

/// Texture, tex data stored in GPU memory (VRAM)
pub const Texture = extern struct {
    /// OpenGL texture id
    id: c_uint = 0,
    /// Texture base width
    width: c_int = 0,
    /// Texture base height
    height: c_int = 0,
    /// Mipmap levels, 1 by default
    mipmaps: c_int = 0,
    /// Data format (PixelFormat type)
    format: c_int = 0,

    pub extern fn DrawTextureRec(texture: Texture, source: Rectangle, position: Vector2, tint: Color) void;
    /// Draw a part of a texture defined by a rectangle
    pub fn drawRec(self: Texture, source: Rectangle, position: Vector2, tint: Color) void {
        self.DrawTextureRec(source, position, tint);
    }
};

/// RenderTexture, fbo for texture rendering
pub const RenderTexture = extern struct {
    /// OpenGL framebuffer object id
    id: c_uint = 0,
    /// Color buffer attachment texture
    texture: Texture = .{},
    /// Depth buffer attachment texture
    depth: Texture = .{},

    extern fn BeginTextureMode(target: RenderTexture) void;
    /// Begin drawing to render texture
    pub fn beginMode(self: RenderTexture) void {
        self.BeginTextureMode();
    }

    extern fn EndTextureMode() void;
    /// Ends drawing to render texture
    pub fn endMode() void {
        EndTextureMode();
    }

    pub extern fn LoadRenderTexture(width: c_int, height: c_int) RenderTexture;
    /// Load texture for rendering (framebuffer)
    pub fn load(width: i32, height: i32) RenderTexture {
        return LoadRenderTexture(width, height);
    }

    pub extern fn UnloadRenderTexture(target: RenderTexture) void;
    /// Unload render texture from GPU memory (VRAM)
    pub fn unload(self: RenderTexture) void {
        self.UnloadRenderTexture();
    }
};

/// System/Window config flags
/// By default all flags are set to false
pub const ConfigFlags = packed struct(c_uint) {
    _padding0: u1 = 0,
    fullscreen_mode: bool = false,
    window_resizable: bool = false,
    window_undecorated: bool = false,
    window_transparent: bool = false,
    msaa_4x_hint: bool = false,
    vsync_hint: bool = false,
    window_hidden: bool = false,
    window_always_run: bool = false,
    window_minimized: bool = false,
    window_maximized: bool = false,
    window_unfocused: bool = false,
    window_topmost: bool = false,
    window_highdpi: bool = false,
    window_mouse_passthrough: bool = false,
    borderless_windowed_mode: bool = false,
    interlaced_hint: bool = false,
    _padding1: u15 = 0,

    extern fn SetConfigFlags(flags: c_uint) void;
    /// Setup init configuration flags (view FLAGS)
    pub fn set(self: ConfigFlags) void {
        SetConfigFlags(@bitCast(self));
    }
};

/// Keyboard keys (US keyboard layout)
/// NOTE: Use GetKeyPressed() to allow redefining
/// required keys for alternative layouts
pub const KeyboardKey = enum(c_int) {
    /// Key: NULL, used for no key pressed
    null = 0,
    // Alphanumeric keys
    /// Key: '
    apostrophe = 39,
    /// Key: ,
    comma = 44,
    /// Key: -
    minus = 45,
    /// Key: .
    period = 46,
    /// Key: /
    slash = 47,
    /// Key: 0
    zero = 48,
    /// Key: 1
    one = 49,
    /// Key: 2
    two = 50,
    /// Key: 3
    three = 51,
    /// Key: 4
    four = 52,
    /// Key: 5
    five = 53,
    /// Key: 6
    six = 54,
    /// Key: 7
    seven = 55,
    /// Key: 8
    eight = 56,
    /// Key: 9
    nine = 57,
    /// Key: ;
    semicolon = 59,
    /// Key: =
    equal = 61,
    /// Key: A | a
    a = 65,
    /// Key: B | b
    b = 66,
    /// Key: C | c
    c = 67,
    /// Key: D | d
    d = 68,
    /// Key: E | e
    e = 69,
    /// Key: F | f
    f = 70,
    /// Key: G | g
    g = 71,
    /// Key: H | h
    h = 72,
    /// Key: I | i
    i = 73,
    /// Key: J | j
    j = 74,
    /// Key: K | k
    k = 75,
    /// Key: L | l
    l = 76,
    /// Key: M | m
    m = 77,
    /// Key: N | n
    n = 78,
    /// Key: O | o
    o = 79,
    /// Key: P | p
    p = 80,
    /// Key: Q | q
    q = 81,
    /// Key: R | r
    r = 82,
    /// Key: S | s
    s = 83,
    /// Key: T | t
    t = 84,
    /// Key: U | u
    u = 85,
    /// Key: V | v
    v = 86,
    /// Key: W | w
    w = 87,
    /// Key: X | x
    x = 88,
    /// Key: Y | y
    y = 89,
    /// Key: Z | z
    z = 90,
    /// Key: [
    left_bracket = 91,
    /// Key: '\'
    backslash = 92,
    /// Key: ]
    right_bracket = 93,
    /// Key: `
    grave = 96,
    // Function keys
    /// Key: Space
    space = 32,
    /// Key: Esc
    escape = 256,
    /// Key: Enter
    enter = 257,
    /// Key: Tab
    tab = 258,
    /// Key: Backspace
    backspace = 259,
    /// Key: Ins
    insert = 260,
    /// Key: Del
    delete = 261,
    /// Key: Cursor right
    right = 262,
    /// Key: Cursor left
    left = 263,
    /// Key: Cursor down
    down = 264,
    /// Key: Cursor up
    up = 265,
    /// Key: Page up
    page_up = 266,
    /// Key: Page down
    page_down = 267,
    /// Key: Home
    home = 268,
    /// Key: End
    end = 269,
    /// Key: Caps lock
    caps_lock = 280,
    /// Key: Scroll down
    scroll_lock = 281,
    /// Key: Num lock
    num_lock = 282,
    /// Key: Print screen
    print_screen = 283,
    /// Key: Pause
    pause = 284,
    /// Key: F1
    f1 = 290,
    /// Key: F2
    f2 = 291,
    /// Key: F3
    f3 = 292,
    /// Key: F4
    f4 = 293,
    /// Key: F5
    f5 = 294,
    /// Key: F6
    f6 = 295,
    /// Key: F7
    f7 = 296,
    /// Key: F8
    f8 = 297,
    /// Key: F9
    f9 = 298,
    /// Key: F10
    f10 = 299,
    /// Key: F11
    f11 = 300,
    /// Key: F12
    f12 = 301,
    /// Key: Shift left
    left_shift = 340,
    /// Key: Control left
    left_control = 341,
    /// Key: Alt left
    left_alt = 342,
    /// Key: Super left
    left_super = 343,
    /// Key: Shift right
    right_shift = 344,
    /// Key: Control right
    right_control = 345,
    /// Key: Alt right
    right_alt = 346,
    /// Key: Super right
    right_super = 347,
    /// Key: KB menu
    kb_menu = 348,
    // Keypad keys
    /// Key: Keypad 0
    kp_0 = 320,
    /// Key: Keypad 1
    kp_1 = 321,
    /// Key: Keypad 2
    kp_2 = 322,
    /// Key: Keypad 3
    kp_3 = 323,
    /// Key: Keypad 4
    kp_4 = 324,
    /// Key: Keypad 5
    kp_5 = 325,
    /// Key: Keypad 6
    kp_6 = 326,
    /// Key: Keypad 7
    kp_7 = 327,
    /// Key: Keypad 8
    kp_8 = 328,
    /// Key: Keypad 9
    kp_9 = 329,
    /// Key: Keypad .
    kp_decimal = 330,
    /// Key: Keypad /
    kp_divide = 331,
    /// Key: Keypad *
    kp_multiply = 332,
    /// Key: Keypad -
    kp_subtract = 333,
    /// Key: Keypad +
    kp_add = 334,
    /// Key: Keypad Enter
    kp_enter = 335,
    /// Key: Keypad =
    kp_equal = 336,
    // Android key buttons
    /// Key: Android back button
    back = 4,
    /// Key: Android menu button
    menu = 5,
    /// Key: Android volume up button
    volume_up = 24,
    /// Key: Android volume down button
    volume_down = 25,
};

/// Window-related functions
pub const Window = struct {
    extern fn InitWindow(width: c_int, height: c_int, title: [*:0]const u8) void;
    /// Initialize window and OpenGL context
    pub fn init(width: i32, height: i32, title: [:0]const u8) void {
        InitWindow(width, height, title.ptr);
    }

    extern fn CloseWindow() void;
    /// Close window and unload OpenGL context
    pub fn close() void {
        CloseWindow();
    }

    extern fn WindowShouldClose() bool;
    /// Check if application should close (KEY_ESCAPE pressed or windows close icon clicked)
    pub fn shouldClose() bool {
        return WindowShouldClose();
    }

    pub extern fn IsWindowResized() bool;
    /// Check if window has been resized last frame
    pub fn isResized() bool {
        return IsWindowResized();
    }
};

extern fn GetScreenWidth() c_int;
/// Get current screen width
pub fn getScreenWidth() i32 {
    return GetScreenWidth();
}
extern fn GetScreenHeight() c_int;
/// Get current screen height
pub fn getScreenHeight() i32 {
    return GetScreenHeight();
}

extern fn BeginDrawing() void;
/// Setup canvas (framebuffer) to start drawing
pub const beginDrawing = BeginDrawing;

extern fn EndDrawing() void;
/// End canvas drawing and swap buffers (double buffering)
pub const endDrawing = EndDrawing;

extern fn SetTargetFPS(fps: c_int) void;
/// Set target FPS (maximum)
pub fn setTargetFPS(fps: i32) void {
    SetTargetFPS(fps);
}

pub const Mouse = struct {
    const Button = enum(c_int) {
        left = 0,
        right = 1,
        middle = 2,
        side = 3,
        extra = 4,
        forward = 5,
        back = 6,
    };

    extern fn IsMouseButtonPressed(button: c_int) bool;
    /// Check if a mouse button has been pressed once
    pub fn isButtonPressed(button: Button) bool {
        return IsMouseButtonPressed(@intFromEnum(button));
    }

    extern fn IsMouseButtonDown(button: c_int) bool;
    /// Check if a mouse button is being pressed
    pub fn isButtonDown(button: Button) bool {
        return IsMouseButtonDown(@intFromEnum(button));
    }

    extern fn IsMouseButtonReleased(button: c_int) bool;
    /// Check if a mouse button has been released once
    pub fn isButtonReleased(button: Button) bool {
        return IsMouseButtonReleased(@intFromEnum(button));
    }

    extern fn GetMousePosition() Vector2;
    /// Get mouse position XY
    pub const getPosition = GetMousePosition;

    extern fn GetMouseWheelMove() f32;
    /// Get mouse wheel movement for X or Y, whichever is larger
    pub const getWheelMove = GetMouseWheelMove;
};

extern fn DrawText(text: [*:0]const u8, posX: c_int, posY: c_int, fontSize: c_int, color: Color) void;
/// Draw text (using default font)
pub fn drawText(text: [*:0]const u8, position_x: i32, position_y: i32, font_size: i32, color: Color) void {
    DrawText(text, position_x, position_y, font_size, color);
}

extern fn MeasureText(text: [*:0]const u8, fontSize: c_int) c_int;
/// Measure string width for default font
pub fn measureText(text: [*:0]const u8, font_size: i32) i32 {
    return MeasureText(text, font_size);
}

pub extern fn DrawLine(startPosX: c_int, startPosY: c_int, endPosX: c_int, endPosY: c_int, color: Color) void;
/// Draw a line
pub fn drawLine(start_position_x: i32, start_position_y: i32, end_position_x: i32, end_position_y: i32, color: Color) void {
    DrawLine(start_position_x, start_position_y, end_position_x, end_position_y, color);
}

extern fn GetCharPressed() c_int;
/// Get char pressed (unicode), call it multiple times for chars queued, returns 0 when the queue is empty
pub fn getCharPressed() i32 {
    return GetCharPressed();
}
