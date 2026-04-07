const std = @import("std");
const cast = std.math.lossyCast;

const rl = @import("raylib");

var selected: ?usize = null;
var current_index: usize = 0;
var max_index: usize = 0;
pub var mouse_position: rl.Vector2 = .{};
var mouse_down = false;
pub var mouse_released = false;
var enter_down = false;
var enter_released = false;
pub var up_pressed = false;
pub var down_pressed = false;
pub var escape_pressed = false;

pub fn getIndex() usize {
    const index = current_index;
    current_index += 1;
    return index;
}

pub fn isSelected(index: usize) bool {
    if (selected) |s| {
        if (index == s) {
            return true;
        }
    }
    return false;
}

pub fn select(index: usize) void {
    selected = index;
}

pub fn frameStart() void {
    mouse_position = rl.Mouse.getPosition();
    mouse_down = rl.Mouse.isButtonDown(.left);
    mouse_released = rl.Mouse.isButtonReleased(.left);
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
        if (selected) |*s| {
            if (s.* == 0 and move == -1) {
                selected = null;
            } else if (s.* == (max_index - 1) and move == 1) {
                selected = null;
            } else {
                s.* = cast(usize, cast(isize, s.*) + move);
            }
        } else {
            if (move == -1) {
                selected = max_index - 1;
            } else {
                selected = 0;
            }
        }
    }
    if (selected) |*s| {
        if (s.* >= max_index) selected = null;
    }
}

pub fn frameEnd() void {
    UI.offset = .{};
    UI.sizes = .{};
    _ = UI.arena.reset(.retain_capacity);
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
    var arena: std.heap.ArenaAllocator = undefined;
    pub var allocator: std.mem.Allocator = undefined;

    pub fn init() void {
        UI.arena = .init(std.heap.page_allocator);
        UI.allocator = arena.allocator();
    }

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
    pub const border_color: rl.Color = .black;
    pub const font_size = 20;
    pub const margin = 1;
};

fn startElement(sizes: rl.Vector2, child_gap: f32) struct { rl.Vector2, rl.Rectangle } {
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
    return .{ offset, rect };
}

fn endElement(offset: rl.Vector2, sizes: rl.Vector2) void {
    UI.offset = offset;
    UI.sizes = sizes;
}

const Label = struct {
    rect: rl.Rectangle,
    offset: rl.Vector2,
    sizes: rl.Vector2,
    border: f32,
    padding: f32,
    font_size: i32,
    text: [:0]const u8,

    fn start(text: [:0]const u8, width: ?f32, padding: f32, border: f32, font_size: i32, child_gap: f32) Label {
        const sizes: rl.Vector2 = .{
            .x = width orelse (padding + border) * 2 + cast(
                f32,
                rl.measureText(
                    text,
                    font_size,
                ),
            ),
            .y = (padding + border) * 2 + cast(f32, font_size),
        };
        const offset, const rect = startElement(sizes, child_gap);
        return .{
            .rect = rect,
            .sizes = sizes,
            .offset = offset,
            .border = border,
            .text = text,
            .padding = padding,
            .font_size = font_size,
        };
    }

    fn end(
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
        endElement(self.offset, self.sizes);
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

const ButtonOptions = struct {
    background: rl.Color = .gray,
    border: f32 = Style.border,
    border_color: rl.Color = Style.border_color,
    width: ?f32 = null,
    padding: f32 = Style.padding,
    child_gap: f32 = Style.child_gap,
    font_size: i32 = 20,
};

pub fn button(text: [:0]const u8, options: ButtonOptions) bool {
    var self = Label.start(
        text,
        options.width,
        options.padding,
        options.border,
        options.font_size,
        options.child_gap,
    );
    var background: rl.Color = options.background;
    var click = false;
    if (mouse_position.checkCollisionRec(self.rect)) {
        if (mouse_down) {
            background = .dark_gray;
        } else if (mouse_released) {
            click = true;
        } else {
            background = .lime;
        }
    }

    const index = getIndex();
    const is_selected = isSelected(index);

    if (is_selected) {
        if (enter_down) {
            background = .dark_gray;
        } else if (enter_released) {
            click = true;
        }
    }

    const border_color: rl.Color = if (is_selected) .blue else options.border_color;
    self.end(background, .black, border_color);
    return click;
}

const InputOptions = struct {
    background: rl.Color = .white,
    padding: f32 = Style.padding,
    child_gap: f32 = Style.child_gap,
    font_size: i32 = 20,
    border: f32 = Style.border,
};

pub fn input(
    allocator: std.mem.Allocator,
    placholder: [:0]const u8,
    text: *std.ArrayList(u8),
    width: f32,
    options: InputOptions,
) !bool {
    const input_text: [:0]const u8, const foreground: rl.Color = blk: {
        if (text.items.len > 0) {
            break :blk .{ try UI.allocator.dupeSentinel(u8, text.items, 0), .black };
        } else {
            break :blk .{ placholder, .gray };
        }
    };
    const self = Label.start(
        input_text,
        width,
        options.padding,
        options.border,
        options.font_size,
        options.child_gap,
    );
    var edited = false;

    const index = getIndex();

    if (mouse_position.checkCollisionRec(self.rect)) {
        if (mouse_released) {
            selected = index;
        }
    }

    const is_selected = isSelected(index);

    if (is_selected) {
        var char = rl.getCharPressed();
        while (char != 0) : (char = rl.getCharPressed()) {
            try text.append(allocator, @intCast(char));
            edited = true;
        }

        if (rl.Key.isReleased(.backspace)) {
            _ = text.pop();
            edited = true;
        }
    }

    const border_color: rl.Color = if (is_selected) .blue else .black;
    self.end(options.background, foreground, border_color);
    return edited;
}
