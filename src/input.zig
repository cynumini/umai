const std = @import("std");
const cast = std.math.lossyCast;
const rl = @import("raylib");
const ui = @import("ui.zig");
const UI = ui.UI;
const Style = ui.Style;

pub const Text = struct {
    placeholder: []i32,
    data: std.ArrayList(i32),
    position: usize = 0,
    timer: f64 = 0,

    pub fn init(allocator: std.mem.Allocator, placeholder: [:0]const u8, initial_text: ?[:0]const u8) !Text {
        var data: std.ArrayList(i32) = .empty;
        if (initial_text) |it| {
            const raw_data = rl.loadCodepoints(it);
            defer rl.unloadCodepoints(raw_data);
            try data.appendSlice(allocator, raw_data);
        }
        return .{ .placeholder = rl.loadCodepoints(placeholder), .data = data };
    }

    pub fn deinit(self: *Text, allocator: std.mem.Allocator) void {
        rl.unloadCodepoints(self.placeholder);
        self.data.deinit(allocator);
    }

    pub fn set(self: *Text, allocator: std.mem.Allocator, text: [:0]const u8) !void {
        self.data.clearRetainingCapacity();
        const codepoints = rl.loadCodepoints(text);
        defer rl.unloadCodepoints(codepoints);
        try self.data.appendSlice(allocator, codepoints);
    }

    pub fn move(self: *Text, direction: enum { forward, backward }) void {
        self.timer = 0;
        switch (direction) {
            .forward => {
                if (self.position < self.data.items.len) self.position += 1;
            },
            .backward => {
                if (self.position > 0) self.position -= 1;
            },
        }
    }

    pub fn move_to_index(self: *Text, index: usize) void {
        self.timer = 0;
        if (self.data.items.len == 0) return;
        if (index <= self.data.items.len) self.position = index;
    }

    pub fn last(self: *Text) void {
        self.timer = 0;
        if (self.data.items.len > 0) {
            self.position = self.data.items.len;
        }
    }

    // Caller own memory
    pub fn get(self: *Text, allocator: std.mem.Allocator) ![:0]const u8 {
        const result = rl.loadUTF8(self.data.items);
        defer rl.unloadUTF8(result);
        return allocator.dupeSentinel(u8, result, 0);
    }

    pub fn add(self: *Text, allocator: std.mem.Allocator, char: i32, position: usize) !void {
        try self.data.insert(allocator, position, char);
        self.move(.forward);
    }

    pub fn delete(self: *Text, position: usize, need_mode: bool) !void {
        if (position < self.data.items.len) {
            _ = self.data.orderedRemove(position);
            if (need_mode or self.position > self.data.items.len) {
                self.move(.backward);
            }
        }
    }

    pub fn isBlinkTime(self: *Text) bool {
        const max: f64 = 1;
        self.timer += rl.getFrameTime();
        var result = false;
        if (self.timer > max / 2) {
            result = true;
        }
        if (self.timer > max) {
            self.timer = 0;
        }
        return result;
    }
};

const InputOptions = struct {
    background: rl.Color = .white,
    padding: f32 = Style.padding,
    child_gap: f32 = Style.child_gap,
    border: f32 = Style.border,
};

fn isKeyPressedRepeat(key: rl.Key) bool {
    return rl.Key.isPressed(key) or rl.Key.isPressedRepeat(key);
}

pub fn input(
    allocator: std.mem.Allocator,
    text: *Text,
    width: f32,
    options: InputOptions,
) !bool {
    const padding_and_border = Style.border + Style.padding;
    const sizes = rl.Vector2{
        .x = width,
        .y = Style.font_size + padding_and_border * 2,
    };
    const rect = ui.startElement(sizes, options.child_gap);

    const index = ui.getIndex();
    const is_active = ui.isActive(index);

    if (ui.onMousePressed(rect)) {
        ui.activate(index);
        const relative = ui.getRelativeMousePosition(rect.toVector2());
        var char_position: f32 = padding_and_border - 2;
        var prev_char_position = char_position;

        for (text.data.items, 0..) |codepoint, new_position| {
            prev_char_position = char_position;
            const codepoint_width = ui.getCharWidth(i32, rl.getFontDefault(), codepoint) + 2;
            char_position += codepoint_width;
            const char_start = prev_char_position;
            const char_mid = char_position - codepoint_width / 2;
            const char_end = char_position;
            if (relative.x >= char_start and relative.x < char_mid) {
                text.move_to_index(new_position);
                break;
            } else if (relative.x >= char_mid and relative.x < char_end) {
                text.move_to_index(new_position + 1);
                break;
            }
        } else {
            if (relative.x < padding_and_border - 2) {
                text.move_to_index(0);
            } else {
                text.last();
            }
        }
    }

    const border_color = if (is_active) Style.active else Style.foreground;

    const codepoints: []const i32, const foreground = blk: {
        if (!is_active and text.data.items.len == 0) {
            break :blk .{ text.placeholder, Style.inactive };
        } else if (text.data.items.len == 0) {
            break :blk .{ &.{}, Style.foreground };
        } else {
            break :blk .{ text.data.items, Style.foreground };
        }
    };

    rect.draw(Style.background);
    rect.drawLines(Style.border, border_color);
    rl.getFontDefault().drawTextCodepoints(
        codepoints,
        .{ .x = rect.x + padding_and_border, .y = rect.y + padding_and_border },
        Style.font_size,
        2,
        foreground,
    );

    if (is_active) {
        var caret = rect.toVector2().addValue(padding_and_border).toRectangle(2, Style.font_size);
        caret.x -= 2;

        var char = rl.getCharPressed();

        for (0..text.position) |i| {
            caret.x += ui.getCharWidth(i32, rl.getFontDefault(), text.data.items[i]) + 2;
        }

        while (char != 0) : (char = rl.getCharPressed()) {
            try text.add(allocator, char, text.position);
        }

        if (isKeyPressedRepeat(.backspace)) {
            if (text.position > 0) {
                try text.delete(text.position - 1, true);
            }
        }

        if (isKeyPressedRepeat(.delete)) {
            try text.delete(text.position, false);
        }

        if (isKeyPressedRepeat(.right)) text.move(.forward);
        if (isKeyPressedRepeat(.left)) text.move(.backward);

        if (!text.isBlinkTime()) {
            caret.draw(Style.foreground);
        }
    } else {}
    return false;
}
