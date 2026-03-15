const std = @import("std");
const rl = @import("raylib");

const ui = @import("ui.zig");

const assert = std.debug.assert;

pub fn main() !void {
    var width: u32 = 1280;
    var height: u32 = 720;
    var scroll: u32 = 0;

    rl.set_config_flags(.{ .window_resizable = true });
    const window = rl.Window.init(width, height, "umai");
    defer window.deinit();

    while (!window.should_close()) {
        // update
        if (window.is_resized()) {
            width = window.get_width();
            height = window.get_height();
        }
        try ui.begin(width, height, .{ .background = .red });
        {
            try ui.container_begin(.{ .background = .green, .width = .grow, .height = .grow });
            {
                try ui.label("Yes, I am!", .{});
                try ui.container_begin(.{
                    .background = .blue,
                    .width = .{ .fixed = 100 },
                    .height = .{ .fixed = 100 },
                });
                ui.container_end();
                try ui.scroll_view_begin(&scroll, .{ .width = .grow, .height = .grow });
                {
                    try ui.container_begin(.{ .direction = .top_to_bottom });
                    for (0..500) |i| try ui.label_fmt("Hello, number {}!", .{i + 1}, .{});
                    ui.container_end();
                }
                ui.scroll_view_end();
            }
            ui.container_end();
        }
        ui.end();
        // draw
        rl.begin_drawing();
        rl.clear_background(rl.Color.white);
        ui.draw();
        rl.end_drawing();
    }
}
