const std = @import("std");
const rl = @import("raylib");

const UI = @import("ui.zig");

const assert = std.debug.assert;

fn on_update(ui: *UI, node: *UI.Node, data: ?*anyopaque) void {
    _ = ui;
    _ = node;
    _ = data;
    std.debug.print("my callback\n", .{});
}

pub fn main() !void {
    var width: u32 = 1280;
    var height: u32 = 720;
    //var scroll: i32 = 0;

    rl.ConfigFlags.set(.{ .window_resizable = true });
    const window = rl.Window.init(width, height, "umai");
    defer window.deinit();

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
        });
        try ui.begin(allocator, .{ .background = .blue });
        try ui.label(allocator, "Add a food", .{
            .on_update = on_update,
        });
        try ui.end(allocator);
        try ui.end(allocator);

        // draw
        rl.beginDrawing();
        rl.clearBackground(.light_gray);
        try ui.draw();
        rl.endDrawing();
        break;
    }
}
