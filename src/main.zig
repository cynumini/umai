const std = @import("std");
const rl = @import("raylib");

const UI = @import("ui.zig");

const assert = std.debug.assert;

// fn callback(_: *UI, _: *UI.Node, data: ?*anyopaque) void {
//     if (data) |d| {
//         const visible: *bool = @ptrCast(@alignCast(d));
//         visible.* = !visible.*;
//     }
// }

pub fn main() !void {
    var width: u32 = 1280;
    var height: u32 = 720;

    //var scroll: i32 = 0;
    var tab_index: usize = 0;

    rl.ConfigFlags.set(.{ .window_resizable = true });
    const window = rl.Window.init(width, height, "umai");
    defer window.deinit();

    rl.setTargetFPS(60);

    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    var v = rl.RenderTexture.init(0, 0);
    defer v.deinit();

    while (!window.shouldClose()) {
        var ui = UI{};
        defer _ = arena.reset(.retain_capacity);

        // update
        if (window.isResized()) {
            width = rl.getScreenWidth();
            height = rl.getScreenHeight();
        }

        try ui.begin(allocator, .{
            .width = .{ .fixed = width },
            .height = .{ .fixed = height },
            .direction = .top_to_bottom,
        });
        try ui.button(allocator, "add a food", .{});

        var tc = try ui.tabContainerBegin(allocator, &tab_index);
        {
            try ui.tabBegin(allocator, &tc, "main", .{
                .background = .red,
            });
            try ui.tabEnd(allocator);
            inline for (&.{ rl.Color.green, rl.Color.blue, rl.Color{ .r = 0, .g = 255, .b = 255, .a = 255 } }) |color| {
                try ui.tabBegin(allocator, &tc, "new food", .{
                    .background = color,
                });
                try ui.tabEnd(allocator);
            }
        }
        try ui.tabContainerEnd(allocator, tc);

        try ui.end(allocator);

        // draw
        rl.beginDrawing();
        rl.clearBackground(.light_gray);
        try ui.draw();
        rl.endDrawing();
    }
}
