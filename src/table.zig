const std = @import("std");
const cast = std.math.lossyCast;

const rl = @import("raylib");

const State = @import("state.zig");
const ui = @import("ui.zig");
const Style = ui.Style;

pub fn table(rect: rl.Rectangle, state: *State) !void {
    var cells_widths: [4]i32 = .{ 0, 0, 0, 0 };
    for (state.foods_str.items) |food| {
        for (food, 0..) |cell, i| {
            cells_widths[i] = @max(cells_widths[i], rl.measureText(cell, Style.font_size));
        }
    }
    var rectangle: rl.Rectangle = .{
        .x = rect.x + Style.padding,
        .y = rect.y + Style.padding,
        .width = Style.margin,
        .height = Style.margin + cast(f32, state.foods_str.items.len * (Style.font_size + Style.padding * 2 + Style.margin)),
    };
    for (cells_widths) |w| {
        rectangle.width += cast(f32, w) + Style.padding * 2 + Style.margin;
    }

    const row_height = Style.font_size + Style.padding * 2 + Style.margin;

    if (state.table_current_row) |current| {
        const selected_background = rl.Rectangle{
            .x = rectangle.x,
            .y = rectangle.y + row_height + (row_height * cast(f32, current)),
            .width = rectangle.width,
            .height = row_height,
        };
        selected_background.draw(.blue);
    }

    var y: i32 = @intFromFloat(rectangle.y);
    for (state.foods_str.items, 0..) |food, i| {
        var x: i32 = @intFromFloat(rectangle.x);
        const start_h: rl.Vector2 = .{
            .x = rectangle.x,
            .y = cast(f32, y + Style.font_size) + Style.padding * 2 + Style.margin,
        };
        start_h.drawLine(
            .{ .y = start_h.y, .x = rectangle.x + rectangle.width },
            .black,
        );
        for (food, 0..) |cell, j| {
            rl.drawText(
                cell,
                x + Style.padding + Style.margin,
                y + Style.padding + Style.margin,
                Style.font_size,
                .black,
            );
            if (i == 0) {
                const start_v: rl.Vector2 = .{
                    .x = cast(f32, x + cells_widths[j]) + (Style.padding + Style.margin) * 2,
                    .y = rectangle.y,
                };
                start_v.drawLine(
                    .{ .x = start_v.x, .y = rectangle.y + rectangle.height },
                    .black,
                );
            }
            x += cells_widths[j] + Style.padding * 2 + Style.margin;
        }
        y += row_height;
    }

    const index = ui.getIndex();
    const is_selected = ui.isActive(index);
    const border_color: rl.Color = if (is_selected) .blue else .black;
    rectangle.drawLines(Style.border, border_color);

    if (ui.onMousePressed(rectangle)) {
        const relative = ui.mouse_position.subtract(rectangle.toVector2());
        const i: usize = @intFromFloat(relative.y / row_height);
        if (i > 0) {
            state.table_current_row = i - 1;
            ui.activate(index);
        }
    }

    if (is_selected) {
        const move = @as(isize, @intFromBool(ui.down_pressed)) - @as(isize, @intFromBool(ui.up_pressed));
        const max_index = state.foods_str.items.len - 1;
        if (move != 0) {
            if (state.table_current_row) |value| {
                state.table_current_row = @intCast(std.math.clamp(
                    cast(isize, value) + move,
                    0,
                    max_index - 1,
                ));
            } else {
                state.table_current_row = 0;
            }
        }
        if (ui.escape_pressed) state.table_current_row = null;
    }
}
