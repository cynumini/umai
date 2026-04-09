const std = @import("std");
const cast = std.math.lossyCast;

const rl = @import("raylib");

var arena: std.heap.ArenaAllocator = undefined;
pub var allocator: std.mem.Allocator = undefined;

var activated: ?usize = null;
var current_index: usize = 0;
var max_index: usize = 0;
pub var mouse_position: rl.Vector2 = .{};
var mouse_down = false;
var mouse_pressed = false;
var enter_down = false;
var enter_released = false;
pub var up_pressed = false;
pub var down_pressed = false;
pub var escape_pressed = false;
var something_activated = false;

pub var last_rect: rl.Rectangle = .{};

pub fn init() void {
    arena = .init(std.heap.page_allocator);
    allocator = arena.allocator();
}

pub fn deinit() void {
    arena.deinit();
}

pub fn getIndex() usize {
    const index = current_index;
    current_index += 1;
    return index;
}

pub fn isActive(index: usize) bool {
    if (activated) |s| {
        if (index == s) {
            return true;
        }
    }
    return false;
}

pub fn activate(index: ?usize) void {
    something_activated = true;
    activated = index;
}

pub fn onMousePressed(rect: rl.Rectangle) bool {
    return mouse_position.checkCollisionRec(rect) and mouse_pressed;
}

pub fn getRelativeMousePosition(position: rl.Vector2) rl.Vector2 {
    return mouse_position.subtract(position);
}

pub fn frameStart() void {
    something_activated = false;

    mouse_position = rl.Mouse.getPosition();
    mouse_down = rl.Mouse.isButtonDown(.left);
    mouse_pressed = rl.Mouse.isButtonPressed(.left);
    enter_down = rl.Key.isDown(.enter);
    enter_released = rl.Key.isReleased(.enter);
    up_pressed = rl.Key.isPressed(.up);
    down_pressed = rl.Key.isPressed(.down);
    escape_pressed = rl.Key.isPressed(.escape);

    max_index = current_index;
    current_index = 0;
    const move: isize = blk: {
        if (rl.Key.isUp(.left_shift) and rl.Key.isPressed(.tab)) {
            break :blk 1;
        } else if (rl.Key.isDown(.left_shift) and rl.Key.isPressed(.tab)) {
            break :blk -1;
        } else {
            break :blk 0;
        }
    };

    if (move != 0) {
        if (activated) |*s| {
            if (s.* == 0 and move == -1) {
                activated = null;
            } else if (s.* == (max_index - 1) and move == 1) {
                activated = null;
            } else {
                s.* = cast(usize, cast(isize, s.*) + move);
            }
        } else {
            if (move == -1) {
                activated = max_index - 1;
            } else {
                activated = 0;
            }
        }
    }
    if (activated) |*s| {
        if (s.* >= max_index) activated = null;
    }
}

pub fn frameEnd() void {
    UI.offset = .{};
    UI.sizes = .{};

    if (!something_activated and mouse_pressed) {
        activate(null);
    }

    _ = arena.reset(.retain_capacity);
}

pub const UI = struct {
    const Direction = enum { down, left };

    const Origin = enum {
        left_top,
        right_bottom,
    };

    pub var sizes: rl.Vector2 = .{};
    var offset: rl.Vector2 = .{};

    var direction: Direction = .down;
    var origin: Origin = .left_top;

    const StartOptions = struct {
        direction: Direction = .down,
        origin: Origin = .left_top,
    };

    pub fn start(position: rl.Vector2, options: StartOptions) void {
        UI.offset = position;
        UI.sizes = .{};
        UI.direction = options.direction;
        UI.origin = options.origin;
    }
};

pub const Style = struct {
    pub const padding = 6;
    pub const child_gap = 6;
    pub const border = 2;

    pub const clear_color = rl.Color.light_gray;

    pub const font_size = 20;
    pub const margin = 1;
    pub const foreground = rl.Color.black;
    pub const border_color = foreground;
    pub const active = rl.Color.lime;
    pub const inactive = rl.Color.gray;
    pub const background = rl.Color.white;
};

pub fn startElement(sizes: rl.Vector2, child_gap: f32) rl.Rectangle {
    const offset: rl.Vector2 = switch (UI.direction) {
        .down => .{
            .x = UI.offset.x,
            .y = UI.offset.y + (UI.sizes.y + if (UI.sizes.y > 0) child_gap else 0),
        },
        .left => .{
            .x = UI.offset.x - (UI.sizes.x + if (UI.sizes.x > 0) child_gap else 0),
            .y = UI.offset.y,
        },
    };
    const rect: rl.Rectangle = (switch (UI.origin) {
        .left_top => offset,
        .right_bottom => offset.subtract(sizes),
    }).toRectangleV(sizes);

    UI.offset = offset;
    UI.sizes = sizes;

    last_rect = rect;

    return rect;
}

pub const Label = struct {
    rect: rl.Rectangle,
    sizes: rl.Vector2,
    border: f32,
    padding: f32,
    font_size: i32,
    text: []const u8,

    pub fn start(text: []const u8, width: ?f32, padding: f32, border: f32, font_size: i32, child_gap: f32) Label {
        const sizes: rl.Vector2 = .{
            .x = width orelse (padding + border) * 2 + cast(
                f32,
                rl.measureText(
                    u8,
                    text,
                    font_size,
                ),
            ),
            .y = (padding + border) * 2 + cast(f32, font_size),
        };
        const rect = startElement(sizes, child_gap);
        return .{
            .rect = rect,
            .sizes = sizes,
            .border = border,
            .text = text,
            .padding = padding,
            .font_size = font_size,
        };
    }

    pub fn end(
        self: Label,
        background: ?rl.Color,
        foreground: rl.Color,
        border_color: rl.Color,
    ) void {
        if (background) |b| self.rect.draw(b);
        if (self.border > 0) self.rect.drawLines(self.border, border_color);
        rl.drawText(
            self.text,
            @intFromFloat(self.rect.x + self.padding + self.border),
            @intFromFloat(self.rect.y + self.padding + self.border),
            self.font_size,
            foreground,
        );
    }
};

const LabelOptions = struct {
    background: ?rl.Color = null,
    border: f32 = 0,
    width: ?f32 = null,
    padding: f32 = Style.padding,
    child_gap: f32 = Style.child_gap,
    font_size: i32 = 20,
};

pub fn label(text: [:0]const u8, options: LabelOptions) void {
    const self = Label.start(
        text,
        options.width,
        options.padding,
        options.border,
        options.font_size,
        options.child_gap,
    );
    self.end(options.background, .black, .black);
}

pub const ButtonOptions = struct {
    background: rl.Color = .gray,
    border: f32 = Style.border,
    border_color: rl.Color = Style.border_color,
    width: ?f32 = null,
    padding: f32 = Style.padding,
    child_gap: f32 = Style.child_gap,
    font_size: i32 = 20,
};

pub fn button(text: []const u8, options: ButtonOptions) bool {
    const index = getIndex();
    const is_selected = isActive(index);

    var border = options.border;
    var padding = options.padding;

    if (is_selected) {
        if (border == 0) border = 2;
    } else {
        if (border == 0) padding += 2;
    }

    var self = Label.start(
        text,
        options.width,
        padding,
        border,
        options.font_size,
        options.child_gap,
    );
    var background: rl.Color = options.background;
    var click = false;
    if (mouse_position.checkCollisionRec(self.rect)) {
        if (mouse_pressed) {
            click = true;
        } else if (mouse_down) {
            background = .dark_gray;
        } else {
            background = .lime;
        }
    }

    if (is_selected) {
        if (enter_down) {
            background = .dark_gray;
        } else if (enter_released) {
            click = true;
        }
    }

    const border_color: rl.Color = if (is_selected) Style.active else options.border_color;
    self.end(background, .black, border_color);
    return click;
}

pub const input = @import("input.zig").input;
pub const Text = @import("input.zig").Text;

pub fn getCharWidth(comptime T: type, font: rl.Font, codepoint: T) f32 {
    const c: i32 = switch (T) {
        i32 => codepoint,
        u8 => @intCast(codepoint),
        else => unreachable,
    };
    const info = rl.getGlyphInfo(font, c);
    return cast(f32, info.image.width) * cast(f32, Style.font_size) / cast(f32, font.baseSize);
}
