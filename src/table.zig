const std = @import("std");

const rl = @import("raylib");

const UI = @import("ui.zig");

pub const Table = struct {
    children_ids: std.ArrayList(usize) = .empty,
    cells_widths: std.ArrayList(f32) = .empty,
    rows_height: std.ArrayList(f32) = .empty,
};
pub const Row = struct {
    children_ids: std.ArrayList(usize) = .empty,
};

const TableOptions = struct {
    id: []const u8 = "",
    paddings: ?UI.Node.Paddings = null,
    background: ?rl.Color = null,
    foreground: ?rl.Color = null,
    border_size: ?u32 = null,
};

pub fn begin(ui: *UI, allocator: std.mem.Allocator, options: TableOptions) !void {
    ui.current_container = try ui.addNode(allocator, .{
        .id = options.id,
        .type = .{ .table = .{} },
        .paddings = options.paddings orelse ui.style.paddings,
        .background = options.background orelse ui.style.background,
        .foreground = options.foreground orelse ui.style.foreground,
        .event = .{ .data = null, .vtable = .{} },
        .border_size = options.border_size orelse ui.style.border_size,
    });
}
pub fn end(ui: *UI) void {
    ui.current_container = ui.getNode(ui.current_container.?).parent;
}

const RowOptions = struct {
    id: []const u8 = "",
    paddings: ?UI.Node.Paddings = null,
    background: ?rl.Color = null,
    foreground: ?rl.Color = null,
    border_size: ?u32 = null,
};

pub fn rowBegin(ui: *UI, allocator: std.mem.Allocator, options: RowOptions) !void {
    ui.current_container = try ui.addNode(allocator, .{
        .id = options.id,
        .type = .{ .row = .{} },
        .paddings = options.paddings orelse ui.style.paddings,
        .background = options.background orelse ui.style.background,
        .foreground = options.foreground orelse ui.style.foreground,
        .event = .{ .data = null, .vtable = .{} },
        .border_size = options.border_size orelse ui.style.border_size,
    });
}
pub const rowEnd = end;
