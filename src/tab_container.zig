const std = @import("std");

const UI = @import("ui.zig");
const rl = @import("raylib");

pub const Tab = struct {
    id: usize,
    node_id: usize,
    name: [:0]const u8,
    tab_index: *usize,
};

pub const TabContainer = struct {
    tab_index: *usize,
    tabs: std.ArrayList(Tab) = .empty,
    initial_container: ?usize,
};

pub fn begin(self: *UI, allocator: std.mem.Allocator, tab_index: *usize) !TabContainer {
    try self.begin(allocator, .{
        .background = .lime,
        .direction = .top_to_bottom,
        .width = .grow,
        .height = .grow,
    });
    try self.begin(allocator, .{ .background = .yellow, .width = .grow });
    const initial_container = self.current_container;
    try self.end(allocator);
    return .{ .tab_index = tab_index, .initial_container = initial_container };
}

fn tab_button_on_click(_: *UI, _: *UI.Node, data: ?*anyopaque) void {
    if (data) |d| {
        const tab: *Tab = @ptrCast(@alignCast(d));
        tab.tab_index.* = tab.id;
    }
}

const TabOptions = struct {
    background: ?rl.Color = null,
};

pub fn end(self: *UI, allocator: std.mem.Allocator, tab_container: TabContainer) !void {
    for (tab_container.tabs.items) |tab| {
        if (tab.id != tab_container.tab_index.*) {
            self.getNode(tab.node_id).visible = false;
        }
    }
    const tmp = self.current_container;
    self.current_container = tab_container.initial_container;
    for (tab_container.tabs.items) |*tab| {
        try self.button(allocator, tab.name, .{
            .background = if (tab.id == tab_container.tab_index.*) .white else .gray,
            .on_click = tab_button_on_click,
            .data = tab,
        });
    }
    self.current_container = tmp;
    try self.end(allocator);
}

pub fn tabBegin(
    self: *UI,
    allocator: std.mem.Allocator,
    tab_container: *TabContainer,
    name: [:0]const u8,
    options: TabOptions,
) !void {
    try tab_container.tabs.append(allocator, .{
        .id = tab_container.tabs.items.len,
        .node_id = self.nodes.items.len,
        .name = name,
        .tab_index = tab_container.tab_index,
    });
    try self.begin(allocator, .{
        .background = options.background orelse self.style.background,
        .width = .grow,
        .height = .grow,
    });
}

pub fn tabEnd(self: *UI, allocator: std.mem.Allocator) !void {
    try self.end(allocator);
}
