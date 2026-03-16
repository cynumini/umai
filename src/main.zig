const std = @import("std");
const rl = @import("raylib");

const UI = @import("ui.zig");

const assert = std.debug.assert;

pub fn main() !void {
    var width: u32 = 1280;
    var height: u32 = 720;
    var scroll: u32 = 0;

    rl.setConfigFlags(.{ .window_resizable = true });
    const window = rl.Window.init(width, height, "umai");
    defer window.deinit();

    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    var v = rl.RenderTexture.init(0, 0);
    defer v.deinit();

    while (!window.shouldClose()) {
        var ui = UI.init();
        defer _ = arena.reset(.retain_capacity);

        // update
        if (window.isResized()) {
            width = window.getWidth();
            height = window.getHeight();
        }
        try ui.begin(allocator, .{
            .background = .red,
            .width = .{ .fixed = width },
            .height = .{ .fixed = height },
        });
        {
            try ui.begin(allocator, .{ .background = .green, .width = .grow, .height = .grow });
            {
                try ui.label(allocator, "Yes, I am!", .{});
                try ui.begin(allocator, .{
                    .background = .blue,
                    .width = .{ .fixed = 100 },
                    .height = .{ .fixed = 100 },
                });
                try ui.end(allocator);
                try ui.scrollViewBegin(allocator, &scroll, &v, .{ .width = .grow, .height = .grow });
                {
                    try ui.begin(allocator, .{ .direction = .top_to_bottom });
                    for (0..500) |i| try ui.labelFmt(allocator, "Hello, number {}!", .{i + 1}, .{});
                    try ui.end(allocator);
                }
                try ui.scrollViewEnd(allocator);
            }
            try ui.end(allocator);
        }
        try ui.end(allocator);
        // draw
        rl.beginDrawing();
        rl.clearBackground(rl.Color.white);
        try ui.draw();
        rl.endDrawing();
        // break;
    }
}
