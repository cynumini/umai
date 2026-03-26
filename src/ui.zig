const std = @import("std");
const rl = @import("raylib");

const cast = std.math.lossyCast;

const Self = @This();

current_container: ?usize = null,
nodes: std.ArrayList(Node) = .empty,
style: Style = .{},

const Container = struct {
    const Direction = enum { left_to_right, top_to_bottom };
    const Alignment = enum { left_top, left_center, left_bottom, center_top, center, center_bottom, right_top, right_center, right_bottom };
    direction: Direction,
    alignment: Alignment,
    child_gap: u32,
    children_ids: std.ArrayList(usize) = .empty,

    fn totalChildGap(self: Container) u32 {
        const len: u32 = @intCast(self.children_ids.items.len);
        if (len > 0) {
            return (len - 1) * self.child_gap;
        }
        return 0;
    }
};

const ScrollView = struct {
    scroll: *i32,
    child_id: ?usize,
    render_texture: *rl.RenderTexture,
};

const Label = struct {
    text: [:0]const u8,
    font_size: u32,
    wrap: bool,
};

const Event = struct {
    data: ?*anyopaque,
    vtable: VTable,

    const VTable = struct {
        on_update: *const fn (ui: *Self, node: *Node, data: ?*anyopaque) void,
    };

    pub fn on_update(self: Event, ui: *Self, node: *Node) void {
        self.vtable.on_update(ui, node, self.data);
    }
};

pub const Node = struct {
    const Type = union(enum) {
        container: Container,
        label: Label,
        scroll_view: ScrollView,
    };
    const Paddings = struct {
        top: u32,
        left: u32,
        bottom: u32,
        right: u32,

        fn all(value: u32) Paddings {
            return .{
                .top = value,
                .left = value,
                .bottom = value,
                .right = value,
            };
        }
    };
    const Size = union(enum) { fit: void, grow: void, fixed: u32 };

    id: []const u8,
    type: Type,
    width: Size = .fit,
    height: Size = .fit,
    parent: ?usize = null,
    paddings: Paddings,
    border_size: u32,
    background: ?rl.Color,
    foreground: ?rl.Color,
    rectangle: rl.Rectangle = .{},
    event: Event,

    fn calcBaseWidth(self: Node) u32 {
        return self.paddings.left + self.paddings.right + (self.border_size * 2);
    }

    fn calcBaseHeight(self: Node) u32 {
        return self.paddings.top + self.paddings.bottom + (self.border_size * 2);
    }
};

const Style = struct {
    paddings: Node.Paddings = .all(8),
    child_gap: u32 = 8,
    border_size: u32 = 0,
    background: ?rl.Color = null,
    scroll_view_background: rl.Color = .white,
    foreground: rl.Color = .black,
    font_size: u32 = 20,
};

fn getNode(self: *Self, node_id: usize) *Node {
    return &self.nodes.items[node_id];
}

fn addNode(self: *Self, allocator: std.mem.Allocator, node: Node) !usize {
    try self.nodes.append(allocator, node);
    const id = self.nodes.items.len - 1;
    self.getNode(id).parent = self.current_container;
    if (self.current_container) |current_container| {
        var container = self.getNode(current_container);
        switch (container.type) {
            .container => |*value| try value.children_ids.append(allocator, id),
            .scroll_view => |*value| value.child_id = id,
            else => unreachable,
        }
    }
    return id;
}

const ContainerOptions = struct {
    id: []const u8 = "",
    direction: Container.Direction = .left_to_right,
    alignment: Container.Alignment = .left_top,
    paddings: ?Node.Paddings = null,
    child_gap: ?u32 = null,
    border_size: ?u32 = null,
    width: Node.Size = .fit,
    height: Node.Size = .fit,
    background: ?rl.Color = null,
    foreground: ?rl.Color = null,
    data: ?*anyopaque = null,
    on_update: *const fn (ui: *Self, node: *Node, data: ?*anyopaque) void = default_on_update,
};

pub fn begin(self: *Self, allocator: std.mem.Allocator, options: ContainerOptions) !void {
    self.current_container = try self.addNode(allocator, .{
        .id = options.id,
        .type = .{ .container = .{
            .direction = options.direction,
            .alignment = options.alignment,
            .child_gap = options.child_gap orelse self.style.child_gap,
        } },
        .paddings = options.paddings orelse self.style.paddings,
        .border_size = options.border_size orelse self.style.border_size,
        .width = options.width,
        .height = options.height,
        .background = options.background orelse self.style.background,
        .foreground = options.foreground orelse self.style.foreground,
        .event = .{
            .data = options.data,
            .vtable = .{ .on_update = options.on_update },
        },
    });
}

fn fitWidth(self: *Self, node: *Node) !void {
    const base_width: f32 = @floatFromInt(node.calcBaseWidth());
    node.rectangle.width = blk: switch (node.type) {
        .label => |l| if (l.wrap) {
            unreachable;
        } else {
            break :blk cast(f32, rl.measureText(l.text, l.font_size)) + base_width;
        },
        .container => |c| {
            for (c.children_ids.items) |id| {
                const child = self.getNode(id);
                try self.fitWidth(child);
            }
            if (node.width == .fixed) break :blk @floatFromInt(node.width.fixed);
            var width: f32 = 0;
            switch (c.direction) {
                .left_to_right => {
                    width = @floatFromInt(c.totalChildGap());
                    for (c.children_ids.items) |id| {
                        width += self.getNode(id).rectangle.width;
                    }
                },
                .top_to_bottom => {
                    for (c.children_ids.items) |id| {
                        width = @max(width, self.getNode(id).rectangle.width);
                    }
                },
            }
            break :blk base_width + width;
        },
        .scroll_view => |sv| {
            try self.fitHeight(self.getNode(sv.child_id.?));
            break :blk if (node.width == .fixed) @floatFromInt(node.width.fixed) else base_width;
        },
    };
}

fn fitHeight(self: *Self, node: *Node) !void {
    const base_height: f32 = @floatFromInt(node.calcBaseHeight());
    node.rectangle.height = blk: switch (node.type) {
        .label => |l| if (l.wrap) {
            unreachable;
        } else {
            break :blk cast(f32, l.font_size) + base_height;
        },
        .container => |c| {
            for (c.children_ids.items) |id| {
                const child = self.getNode(id);
                try self.fitHeight(child);
            }
            if (node.height == .fixed) break :blk @floatFromInt(node.height.fixed);
            var height: f32 = 0;
            switch (c.direction) {
                .top_to_bottom => {
                    height = @floatFromInt(c.totalChildGap());
                    for (c.children_ids.items) |id| {
                        height += self.getNode(id).rectangle.height;
                    }
                },
                .left_to_right => {
                    for (c.children_ids.items) |id| {
                        height = @max(height, self.getNode(id).rectangle.height);
                    }
                },
            }
            break :blk base_height + height;
        },
        .scroll_view => |sv| {
            try self.fitHeight(self.getNode(sv.child_id.?));
            break :blk if (node.height == .fixed) @floatFromInt(node.height.fixed) else base_height;
        },
    };
}

fn growWidth(self: *Self, allocator: std.mem.Allocator, root: *Node) !void {
    var remaining_width = root.rectangle.width - cast(f32, root.calcBaseWidth());
    var growable = std.ArrayList(*Node).empty;
    defer growable.deinit(allocator);

    const container = if (root.type == .container) &root.type.container else return;

    switch (container.direction) {
        .left_to_right => {
            remaining_width -= @floatFromInt(container.totalChildGap());
            for (container.children_ids.items) |id| {
                const child = self.getNode(id);
                remaining_width -= child.rectangle.width;
                if (child.width == .grow) try growable.append(allocator, child);
            }

            while (remaining_width > 0 and growable.items.len > 0) {
                var smallest = growable.items[0].rectangle.width;
                var second_smallest = std.math.floatMax(@TypeOf(smallest));
                var width_to_add = remaining_width;
                for (growable.items) |child| {
                    if (child.rectangle.width < smallest) {
                        second_smallest = smallest;
                        smallest = child.rectangle.width;
                    } else if (child.rectangle.width > smallest) {
                        second_smallest = @min(second_smallest, child.rectangle.width);
                        width_to_add = second_smallest - smallest;
                    }
                }
                width_to_add = @min(width_to_add, remaining_width / cast(f32, growable.items.len));
                if (width_to_add == 0) break;
                for (growable.items) |child| {
                    if (child.rectangle.width == smallest) {
                        child.rectangle.width += width_to_add;
                        remaining_width -= width_to_add;
                    }
                }
            }
        },
        .top_to_bottom => {
            for (container.children_ids.items) |id| {
                const child = self.getNode(id);
                if (child.width == .grow)
                    child.rectangle.width = @max(remaining_width, child.rectangle.width);
            }
        },
    }
    for (container.children_ids.items) |id| {
        const child = self.getNode(id);
        if (child.type == .container) try self.growWidth(allocator, child);
    }
}

fn growHeight(self: *Self, allocator: std.mem.Allocator, node: *Node) !void {
    var remaining_height = node.rectangle.height - cast(f32, node.calcBaseHeight());
    var growable = std.ArrayList(*Node).empty;
    defer growable.deinit(allocator);

    const container = if (node.type == .container) &node.type.container else return;

    switch (container.direction) {
        .top_to_bottom => {
            remaining_height -= @floatFromInt(container.totalChildGap());
            for (container.children_ids.items) |id| {
                const child = self.getNode(id);
                remaining_height -= child.rectangle.height;
                if (child.height == .grow) try growable.append(allocator, child);
            }

            while (remaining_height > 0 and growable.items.len > 0) {
                var smallest = growable.items[0].rectangle.height;
                var second_smallest = std.math.floatMax(@TypeOf(smallest));
                var height_to_add = remaining_height;
                for (growable.items) |child| {
                    if (child.rectangle.height < smallest) {
                        second_smallest = smallest;
                        smallest = child.rectangle.height;
                    } else if (child.rectangle.height > smallest) {
                        second_smallest = @min(second_smallest, child.rectangle.height);
                        height_to_add = second_smallest - smallest;
                    }
                }
                height_to_add = @min(height_to_add, remaining_height / cast(f32, growable.items.len));
                if (height_to_add == 0) break;
                for (growable.items) |child| {
                    if (child.rectangle.height == smallest) {
                        child.rectangle.height += height_to_add;
                        remaining_height -= height_to_add;
                    }
                }
            }
        },
        .left_to_right => {
            for (container.children_ids.items) |id| {
                const child = self.getNode(id);
                if (child.height == .grow)
                    child.rectangle.height = @max(remaining_height, child.rectangle.height);
            }
        },
    }
    for (container.children_ids.items) |id| {
        const child = self.getNode(id);
        if (child.type == .container) try self.growHeight(allocator, child);
    }
}

fn positionChildren(self: *Self, node: *Node) !void {
    switch (node.type) {
        .container => |c| {
            var left_offset = node.rectangle.x + cast(f32, node.paddings.right + node.border_size);
            var top_offset = node.rectangle.y + cast(f32, node.paddings.top + node.border_size);

            const tcg: f32 = @floatFromInt(c.totalChildGap());
            var remaining_width = node.rectangle.width - tcg;
            var remaining_height = node.rectangle.height - tcg;

            for (c.children_ids.items) |id| {
                const child = self.getNode(id);
                remaining_width -= child.rectangle.width;
                remaining_height -= child.rectangle.height;
            }

            switch (c.direction) {
                .left_to_right => {
                    if (c.alignment == .center) {
                        left_offset += remaining_width / 2;
                    }
                    for (c.children_ids.items) |id| {
                        const child = self.getNode(id);
                        child.rectangle.x += left_offset;
                        if (c.alignment == .center) {
                            child.rectangle.y = (node.rectangle.height - child.rectangle.height) / 2;
                        } else {
                            child.rectangle.y = top_offset;
                        }
                        left_offset += child.rectangle.width + cast(f32, c.child_gap);
                        try self.positionChildren(self.getNode(id));
                    }
                },
                .top_to_bottom => {
                    if (c.alignment == .center) {
                        top_offset += remaining_height / 2;
                    }
                    for (c.children_ids.items) |id| {
                        const child = self.getNode(id);
                        child.rectangle.y += top_offset;
                        if (c.alignment == .center) {
                            child.rectangle.x += (node.rectangle.width - child.rectangle.width) / 2;
                        } else {
                            child.rectangle.x += left_offset;
                        }
                        top_offset += child.rectangle.height + cast(f32, c.child_gap);
                        try self.positionChildren(self.getNode(id));
                    }
                },
            }
        },
        .scroll_view => |sv| {
            const child = self.getNode(sv.child_id.?);
            sv.scroll.* = std.math.clamp(
                sv.scroll.*,
                0,
                cast(i32, child.rectangle.height - node.rectangle.height),
            );
            child.rectangle.y = -cast(f32, sv.scroll.*);
            try self.positionChildren(child);
        },
        else => {},
    }
}

pub fn end(self: *Self, allocator: std.mem.Allocator) !void {
    const container_node = self.getNode(self.current_container.?);
    self.current_container = container_node.parent;
    if (container_node.parent == null) {
        try self.fitWidth(container_node);
        try self.growWidth(allocator, container_node);
        // Wrap text
        try self.fitHeight(container_node);
        try self.growHeight(allocator, container_node);

        try self.positionChildren(container_node);

        // ScrollView render
        var i: usize = self.nodes.items.len;
        while (i > 0) : (i -= 1) {
            const node = self.getNode(i - 1);
            if (node.type == .scroll_view) {
                var sv = node.type.scroll_view;
                const child = self.getNode(sv.child_id.?);
                const width: i32 = @intFromFloat(node.rectangle.width);
                const height: i32 = @intFromFloat(node.rectangle.height);
                if (width > sv.render_texture.texture.width or height > sv.render_texture.texture.height) {
                    sv.render_texture.deinit();
                    sv.render_texture.* = .init(width, height);
                }
                sv.render_texture.begin();
                rl.clearBackground(node.background.?);
                try self.drawNode(child);
                sv.render_texture.end();
            }
        }

        // Update
        container_node.event.on_update(self, container_node);
    }
}

const ScrollViewOptions = struct {
    id: []const u8 = "",
    paddings: ?Node.Paddings = null,
    border_size: ?u32 = null,
    width: Node.Size = .fit,
    height: Node.Size = .fit,
    background: ?rl.Color = null,
    foreground: ?rl.Color = null,
    data: ?*anyopaque = null,
    on_update: *const fn (ui: *Self, node: *Node, data: ?*anyopaque) void = default_on_update,
};

// TODO: add scroll sliderp
pub fn scrollViewBegin(self: *Self, allocator: std.mem.Allocator, scroll: *i32, viewport: *rl.RenderTexture, options: ScrollViewOptions) !void {
    scroll.* -= @intFromFloat(rl.getMouseWheelMove() * 80);
    self.current_container = try self.addNode(allocator, .{
        .id = options.id,
        .type = .{ .scroll_view = .{ .scroll = scroll, .child_id = null, .render_texture = viewport } },
        .paddings = options.paddings orelse self.style.paddings,
        .border_size = options.border_size orelse self.style.border_size,
        .width = options.width,
        .height = options.height,
        .background = options.background orelse self.style.scroll_view_background,
        .foreground = options.foreground orelse self.style.foreground,
        .event = .{
            .data = options.data,
            .vtable = &.{
                .on_update = options.on_update,
            },
        },
    });
}

pub fn scrollViewEnd(self: *Self, allocator: std.mem.Allocator) !void {
    try self.end(allocator);
}

const LabelOptions = struct {
    id: []const u8 = "",
    paddings: ?Node.Paddings = null,
    border_size: ?u32 = null,
    font_size: ?u32 = null,
    background: ?rl.Color = null,
    foreground: ?rl.Color = null,
    wrap: bool = false,
    data: ?*anyopaque = null,
    on_update: *const fn (ui: *Self, node: *Node, data: ?*anyopaque) void = default_on_update,
};

pub fn label(self: *Self, allocator: std.mem.Allocator, text: [:0]const u8, options: LabelOptions) !void {
    _ = try self.addNode(allocator, .{
        .id = options.id,
        .type = .{ .label = .{
            .text = text,
            .wrap = options.wrap,
            .font_size = options.font_size orelse self.style.font_size,
        } },
        .paddings = options.paddings orelse self.style.paddings,
        .border_size = options.border_size orelse self.style.border_size,
        .background = options.background orelse self.style.background,
        .foreground = options.foreground orelse self.style.foreground,
        .event = .{
            .data = options.data,
            .vtable = .{
                .on_update = options.on_update,
            },
        },
    });
}

pub fn labelFmt(self: *Self, allocator: std.mem.Allocator, comptime fmt: []const u8, args: anytype, options: LabelOptions) !void {
    try label(self, allocator, try std.fmt.allocPrintSentinel(allocator, fmt, args, 0), options);
}

// TODO: discard out of screen
pub fn drawNode(self: *Self, node: *Node) !void {
    switch (node.type) {
        .container => |c| {
            if (node.background) |b| {
                node.rectangle.draw(b);
            }
            for (c.children_ids.items) |id| try self.drawNode(self.getNode(id));
        },
        .label => |l| {
            const offest: rl.Vector2 = .{
                .x = node.rectangle.x + cast(f32, node.paddings.left),
                .y = node.rectangle.y + cast(f32, node.paddings.top),
            };
            rl.drawText(
                l.text,
                @intFromFloat(offest.x),
                @intFromFloat(offest.y),
                @intCast(l.font_size),
                node.foreground.?,
            );
        },
        .scroll_view => |sv| {
            const diff: f32 = cast(f32, sv.render_texture.texture.height) - node.rectangle.height;
            sv.render_texture.texture.drawRec(
                .{
                    .x = 0,
                    .y = diff,
                    .width = node.rectangle.width,
                    .height = -node.rectangle.height,
                },
                .{ .x = node.rectangle.x, .y = node.rectangle.y },
                .white,
            );
        },
    }
}

pub fn draw(self: *Self) !void {
    if (self.current_container != null) unreachable;
    try self.drawNode(self.getNode(0));
}

fn default_on_update(self: *Self, node: *Node, data: ?*anyopaque) void {
    _ = data;
    std.debug.print("on update\n", .{});

    // propagate
    switch (node.type) {
        .container => |c| {
            for (c.children_ids.items) |id| {
                const child = self.getNode(id);
                child.event.on_update(self, child);
            }
        },
        .scroll_view => |sv| {
            const child = self.getNode(sv.child_id.?);
            child.event.on_update(self, child);
        },
        .label => {},
    }
}
