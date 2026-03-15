const std = @import("std");
const c = @import("raylib_h.zig");

/// Color, 4 components, R8G8B8A8 (32bit)
pub const Color = c.Color;

pub const Flags = packed struct(c_int) {
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
};

/// Window-related functions
pub const Window = struct {
    const Self = @This();

    /// Initialize window and OpenGL context
    pub fn init(width: u32, height: u32, title: [:0]const u8) Self {
        c.InitWindow(@intCast(width), @intCast(height), title.ptr);
        return .{};
    }

    /// Close window and unload OpenGL context
    pub fn deinit(_: Self) void {
        c.CloseWindow();
    }

    /// Check if application should close (KEY_ESCAPE pressed or windows close icon clicked)
    pub fn should_close(_: Self) bool {
        return c.WindowShouldClose();
    }

    /// Check if window has been resized last frame
    pub fn is_resized(_: Self) bool {
        return c.IsWindowResized();
    }

    /// Get current screen width
    pub fn get_width(_: Self) u32 {
        return @intCast(c.GetScreenWidth());
    }

    /// Get current screen height
    pub fn get_height(_: Self) u32 {
        return @intCast(c.GetScreenHeight());
    }
};

/// Set background color (framebuffer clear color)
pub const clear_background = c.ClearBackground;
/// Setup canvas (framebuffer) to start drawing
pub const begin_drawing = c.BeginDrawing;
/// End canvas drawing and swap buffers (double buffering)
pub const end_drawing = c.EndDrawing;
/// Setup init configuration flags (view FLAGS)
pub fn set_config_flags(flags: Flags) void {
    c.SetConfigFlags(@bitCast(flags));
}
/// Get mouse wheel movement for X or Y, whichever is larger
pub const get_mouse_wheel_move = c.GetMouseWheelMove;
/// Measure string width for default font
pub fn measure_text(text: [*:0]const u8, font_size: u32) u32 {
    return @intCast(c.MeasureText(text, @intCast(font_size)));
}
