const std = @import("std");
const cast = std.math.lossyCast;

const rl = @import("raylib");

const Self = @This();

const UIError = error{
    RootNodeMissing,
};

const NodeID = union(enum) { root: void, id: usize };

current_container: NodeID,
current: NodeID,
nodes: std.ArrayList(Node),
root: ?Node,

pub fn init() Self {
    return .{
        .current_container = .root,
        .current = .root,
        .nodes = .empty,
        .root = null,
    };
}

const Container = struct {
    const Direction = enum { left_to_right, top_to_bottom };
    const Alignment = enum { left_top, left_center, left_bottom, center_top, center, center_bottom, right_top, right_center, right_bottom };
    direction: Direction,
    alignment: Alignment,
    child_gap: u32,
    children: std.ArrayList(usize) = .empty,
};

const ScrollView = struct {
    scroll: *i32,
    child: ?usize,
    viewport: *rl.RenderTexture,
};

const Label = struct {
    text: [:0]const u8,
    font_size: u32,
    wrap: bool,
};

const Node = struct {
    const Type = union(enum) {
        container: Container,
        label: Label,
        scroll_view: ScrollView,
    };
    const Sides = struct {
        top: u32,
        left: u32,
        bottom: u32,
        right: u32,

        fn all(value: u32) Sides {
            return .{
                .top = value,
                .left = value,
                .bottom = value,
                .right = value,
            };
        }
    };
    const Size = union(enum) { fit: void, grow: void, fixed: u32 };
    const Sizes = struct {
        width: Size,
        height: Size,
    };

    sizes: ?Sizes,
    type: Type,
    parent: NodeID,
    paddings: Sides,
    border_size: u32,
    background: ?rl.Color,
    foreground: ?rl.Color,
    rectangle: rl.Rectangle = .{},

    fn calcOwnSize(self: Node, size_type: enum { width, height }) f32 {
        return @floatFromInt(switch (size_type) {
            .width => self.paddings.left + self.paddings.right + (self.border_size * 2),
            .height => self.paddings.top + self.paddings.bottom + (self.border_size * 2),
        });
    }
};

const Style = struct {
    paddings: Node.Sides = .all(8),
    child_gap: u32 = 8,
    border_size: u32 = 0,
    background: ?rl.Color = null,
    foreground: rl.Color = .black,
    font_size: u32 = 20,
};

pub var style: Style = .{};

const RootContainerOptions = struct {
    direction: Container.Direction = .left_to_right,
    alignment: Container.Alignment = .left_top,
    background: ?rl.Color = null,
    foreground: ?rl.Color = null,
    paddings: ?Node.Sides = null,
    child_gap: ?u32 = null,
    border_size: ?u32 = null,
};

fn getNode(self: *Self, node_id: NodeID) !*Node {
    switch (node_id) {
        .root => if (self.root) |*node| return node else return UIError.RootNodeMissing,
        .id => |index| return &self.nodes.items[index],
    }
}

fn addNode(self: *Self, allocator: std.mem.Allocator, node: Node) !NodeID {
    switch (self.current) {
        .root => {
            self.root = node;
            self.current = .{ .id = 0 };
            return .root;
        },
        .id => |index| {
            try self.nodes.append(allocator, node);
            var container = try self.getNode(self.current_container);
            switch (container.type) {
                .container => |*value| try value.children.append(allocator, index),
                .scroll_view => |*value| value.child = index,
                else => unreachable,
            }
            self.current.id += 1;
            return .{ .id = index };
        },
    }
}

const ContainerOptions = struct {
    direction: Container.Direction = .left_to_right,
    alignment: Container.Alignment = .left_top,
    paddings: ?Node.Sides = null,
    child_gap: ?u32 = null,
    border_size: ?u32 = null,
    width: Node.Size = .fit,
    height: Node.Size = .fit,
    background: ?rl.Color = null,
    foreground: ?rl.Color = null,
};

pub fn begin(self: *Self, allocator: std.mem.Allocator, options: ContainerOptions) !void {
    self.current_container = try self.addNode(allocator, .{
        .type = .{ .container = .{
            .direction = options.direction,
            .alignment = options.alignment,
            .child_gap = options.child_gap orelse style.child_gap,
        } },
        .paddings = options.paddings orelse style.paddings,
        .border_size = options.border_size orelse style.border_size,
        .sizes = .{
            .width = options.width,
            .height = options.height,
        },
        .background = options.background orelse style.background,
        .foreground = options.foreground orelse style.foreground,
        .parent = self.current_container,
    });
}

fn fitGeneric(self: *Self, children: std.ArrayList(usize), own_size: f32, comptime size_name: []const u8) !f32 {
    var size: f32 = 0;
    for (children.items) |id| {
        const child = try self.getNode(.{ .id = id });
        size = @max(size, @field(child.rectangle, size_name));
    }
    return own_size + size;
} // TODO: extend it

fn fitWidth(self: *Self, root: *Node) !void {
    const own_width: f32 = root.calcOwnSize(.width);
    root.rectangle.width = blk: switch (root.type) {
        .label => |l| if (l.wrap) {
            unreachable;
        } else {
            break :blk cast(f32, rl.measureText(l.text, l.font_size)) + own_width;
        },
        .container => |c| {
            for (c.children.items) |id| {
                const child = try self.getNode(.{ .id = id });
                try self.fitWidth(child);
            }
            if (root.sizes) |sizes| if (sizes.width == .fixed) break :blk @floatFromInt(sizes.width.fixed);
            switch (c.direction) {
                .left_to_right => {
                    var width = own_width;
                    if (c.children.items.len > 0) {
                        width += cast(f32, (c.children.items.len - 1) * c.child_gap);
                    }
                    for (c.children.items) |id| {
                        const child = try self.getNode(.{ .id = id });
                        width += child.rectangle.width;
                    }
                    break :blk width;
                },
                .top_to_bottom => break :blk try self.fitGeneric(c.children, own_width, "width"),
            }
        },
        .scroll_view => {
            if (root.sizes) |sizes| if (sizes.width == .fixed) break :blk @floatFromInt(sizes.width.fixed);
            break :blk own_width;
        },
    };
}

// fit height don't pass fitHeight through scroll_view
fn fitHeight(self: *Self, root: *Node) !void {
    const own_height: f32 = root.calcOwnSize(.height);
    root.rectangle.height = blk: switch (root.type) {
        .label => |l| if (l.wrap) {
            unreachable;
        } else {
            break :blk cast(f32, l.font_size) + own_height;
        },
        .container => |c| {
            for (c.children.items) |id| {
                const child = try self.getNode(.{ .id = id });
                try self.fitHeight(child);
            }
            if (root.sizes) |sizes| if (sizes.height == .fixed) break :blk @floatFromInt(sizes.height.fixed);
            switch (c.direction) {
                .top_to_bottom => {
                    var height = own_height;
                    if (c.children.items.len > 0) {
                        height += cast(f32, (c.children.items.len - 1) * c.child_gap);
                    }
                    for (c.children.items) |id| {
                        const child = try self.getNode(.{ .id = id });
                        height += child.rectangle.height;
                    }
                    break :blk height;
                },
                .left_to_right => break :blk try self.fitGeneric(c.children, own_height, "height"),
            }
        },
        .scroll_view => |cv| {
            if (cv.child) |child_id| try self.fitHeight(try self.getNode(.{ .id = child_id }));
            if (root.sizes) |sizes| if (sizes.height == .fixed) break :blk @floatFromInt(sizes.height.fixed);
            break :blk own_height;
        },
    };
}

fn growWidth(self: *Self, allocator: std.mem.Allocator, root: *Node) !void {
    var remaining_width = root.rectangle.width - root.calcOwnSize(.width);
    var growable = std.ArrayList(*Node).empty;
    defer growable.deinit(allocator);

    const container = if (root.type == .container) &root.type.container else return;

    switch (container.direction) {
        .left_to_right => {
            if (container.children.items.len > 0) {
                remaining_width -= cast(f32, container.child_gap * (container.children.items.len - 1));
            }
            for (container.children.items) |id| {
                const child = try self.getNode(.{ .id = id });
                remaining_width -= child.rectangle.width;
                if (child.sizes) |sizes| if (sizes.width == .grow) try growable.append(allocator, child);
            }

            while (remaining_width > 0 and growable.items.len > 0) {
                var smallest = growable.items[0].rectangle.width;
                var second_smallest = std.math.floatMax(@TypeOf(smallest));
                var width_to_add = remaining_width;
                for (growable.items) |child| {
                    if (child.rectangle.width < smallest) {
                        second_smallest = smallest;
                        smallest = child.rectangle.width;
                    }
                    if (child.rectangle.width > smallest) {
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
            for (container.children.items) |id| {
                const child = try self.getNode(.{ .id = id });
                if (child.sizes) |sizes| {
                    if (sizes.width == .grow) child.rectangle.width = @max(remaining_width, child.rectangle.width);
                }
            }
        },
    }
    for (container.children.items) |id| {
        const child = try self.getNode(.{ .id = id });
        if (child.type == .container) try self.growWidth(allocator, child);
    }
}

fn growHeight(self: *Self, allocator: std.mem.Allocator, root: *Node) !void {
    var remaining_height = root.rectangle.height - root.calcOwnSize(.height);
    var growable = std.ArrayList(*Node).empty;
    defer growable.deinit(allocator);

    const container = if (root.type == .container) &root.type.container else return;

    switch (container.direction) {
        .top_to_bottom => {
            if (container.children.items.len > 0) {
                remaining_height -= cast(f32, container.child_gap * (container.children.items.len - 1));
            }
            for (container.children.items) |id| {
                const child = try self.getNode(.{ .id = id });
                remaining_height -= child.rectangle.height;
                if (child.sizes) |sizes| if (sizes.height == .grow) try growable.append(allocator, child);
            }

            while (remaining_height > 0 and growable.items.len > 0) {
                var smallest = growable.items[0].rectangle.height;
                var second_smallest = std.math.floatMax(@TypeOf(smallest));
                var height_to_add = remaining_height;
                for (growable.items) |child| {
                    if (child.rectangle.height < smallest) {
                        second_smallest = smallest;
                        smallest = child.rectangle.height;
                    }
                    if (child.rectangle.height > smallest) {
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
            for (container.children.items) |id| {
                const child = try self.getNode(.{ .id = id });
                if (child.sizes) |sizes| {
                    if (sizes.height == .grow) child.rectangle.height = @max(remaining_height, child.rectangle.height);
                }
            }
        },
    }
    for (container.children.items) |id| {
        const child = try self.getNode(.{ .id = id });
        if (child.type == .container) try self.growHeight(allocator, child);
    }
}

fn position(self: *Self, root: *Node) !void {
    var left_offset = root.rectangle.x + cast(f32, root.paddings.right + root.border_size);
    var top_offset = root.rectangle.y + cast(f32, root.paddings.top + root.border_size);

    var remaining_width = root.rectangle.width;
    var remaining_height = root.rectangle.width;

    const container = blk: {
        if (root.type == .container) {
            break :blk &root.type.container;
        } else if (root.type == .scroll_view) {
            if (root.type.scroll_view.child) |id| {
                var child = try self.getNode(.{ .id = id });
                if (child.type == .container) try self.position(child);
            }
            return;
        } else {
            return;
        }
    };

    for (container.children.items) |id| {
        const child = try self.getNode(.{ .id = id });
        remaining_width -= child.rectangle.width;
        remaining_height -= child.rectangle.height;
    }

    switch (container.direction) {
        .left_to_right => {
            if (container.children.items.len > 0) {
                remaining_width -= cast(f32, (container.children.items.len - 1) * container.child_gap);
            }
            if (container.alignment == .center) {
                left_offset += remaining_width / 2;
            }
            for (container.children.items) |id| {
                const child = try self.getNode(.{ .id = id });
                var child_position: rl.Vector2 = .{};
                child_position.x += left_offset;
                child_position.y = top_offset;
                if (container.alignment == .center) {
                    child_position.y += (root.rectangle.height - child.rectangle.height) / 2;
                }
                child.rectangle.x = child_position.x;
                child.rectangle.y = child_position.y;
                left_offset += child.rectangle.width + cast(f32, container.child_gap);
            }
        },
        .top_to_bottom => {
            if (container.children.items.len > 0) {
                remaining_height -= cast(f32, (container.children.items.len - 1) * container.child_gap);
            }
            if (container.alignment == .center) {
                top_offset += remaining_height / 2;
            }
            for (container.children.items) |id| {
                const child = try self.getNode(.{ .id = id });
                var child_position: rl.Vector2 = .{};
                if (container.alignment == .center) {
                    child_position.x += (root.rectangle.width - child.rectangle.width) / 2;
                } else {
                    child_position.x += left_offset;
                }
                child_position.y += top_offset;
                child.rectangle.x = child_position.x;
                child.rectangle.y = child_position.y;
                top_offset += child.rectangle.height + cast(f32, container.child_gap);
            }
        },
    }

    for (container.children.items) |id| {
        const child = try self.getNode(.{ .id = id });
        if (child.type == .container or child.type == .scroll_view) try self.position(child);
    }
}

pub fn end(self: *Self, allocator: std.mem.Allocator) !void {
    if (self.current_container == .root) {
        const root_root = try self.getNode(self.current_container);
        try self.fitWidth(root_root);
        try self.growWidth(allocator, root_root);
        // Wrap text
        try self.fitHeight(root_root);
        try self.growHeight(allocator, root_root);
        try self.position(root_root);

        var i: usize = self.nodes.items.len;
        while (i > 0) : (i -= 1) {
            const child = try self.getNode(.{ .id = i - 1 });
            if (child.type == .scroll_view) {
                var sv = child.type.scroll_view;
                const root = try self.getNode(.{ .id = sv.child.? });
                const width: i32 = @intFromFloat(child.rectangle.width);
                const height: i32 = @intFromFloat(child.rectangle.height);
                if (width > sv.viewport.texture.width or height > sv.viewport.texture.height) {
                    sv.viewport.deinit();
                    sv.viewport.* = rl.RenderTexture.init(width, height);
                }
                var temp: rl.Vector2 = .{ .x = root.rectangle.x, .y = root.rectangle.y };
                sv.scroll.* = std.math.clamp(
                    sv.scroll.*,
                    0,
                    cast(i32, root.rectangle.height - child.rectangle.height),
                );
                root.rectangle.x = 0;
                root.rectangle.y = -cast(f32, sv.scroll.*);
                sv.viewport.begin();
                rl.clearBackground(.white);
                try self.position(root_root);
                try self.drawNode(root);
                sv.viewport.end();
                root.rectangle.x = temp.x;
                root.rectangle.y = temp.y;
            }
        }
    } else {
        self.current_container = (try self.getNode(self.current_container)).parent;
    }
}

const ScrollViewOptions = struct {
    paddings: ?Node.Sides = null,
    border_size: ?u32 = null,
    width: Node.Size = .fit,
    height: Node.Size = .fit,
    background: ?rl.Color = null,
    foreground: ?rl.Color = null,
};

// TODO: add scroll slider
pub fn scrollViewBegin(self: *Self, allocator: std.mem.Allocator, scroll: *i32, viewport: *rl.RenderTexture, options: ScrollViewOptions) !void {
    scroll.* -= @intFromFloat(rl.getMouseWheelMove() * 80);
    self.current_container = try self.addNode(allocator, .{
        .type = .{ .scroll_view = .{ .scroll = scroll, .child = null, .viewport = viewport } },
        .paddings = options.paddings orelse style.paddings,
        .border_size = options.border_size orelse style.border_size,
        .sizes = .{
            .width = options.width,
            .height = options.height,
        },
        .background = options.background orelse style.background,
        .foreground = options.foreground orelse style.foreground,
        .parent = self.current_container,
    });
}

pub fn scrollViewEnd(self: *Self, allocator: std.mem.Allocator) !void {
    try self.end(allocator);
}

const LabelOptions = struct {
    paddings: ?Node.Sides = null,
    border_size: ?u32 = null,
    font_size: ?u32 = null,
    background: ?rl.Color = null,
    foreground: ?rl.Color = null,
    wrap: bool = false,
};

pub fn label(self: *Self, allocator: std.mem.Allocator, text: [:0]const u8, options: LabelOptions) !void {
    _ = try self.addNode(allocator, .{
        .type = .{ .label = .{
            .text = text,
            .wrap = options.wrap,
            .font_size = options.font_size orelse style.font_size,
        } },
        .paddings = options.paddings orelse style.paddings,
        .border_size = options.border_size orelse style.border_size,
        .background = options.background orelse style.background,
        .foreground = options.foreground orelse style.foreground,
        .parent = self.current_container,
        .sizes = null,
    });
}

pub fn labelFmt(self: *Self, allocator: std.mem.Allocator, comptime fmt: []const u8, args: anytype, options: LabelOptions) !void {
    try label(self, allocator, try std.fmt.allocPrintSentinel(allocator, fmt, args, 0), options);
}

// TODO: discard out of screen
pub fn drawNode(self: *Self, root: *Node) !void {
    switch (root.type) {
        .container => |c| {
            if (root.background) |b| {
                root.rectangle.draw(b);
            }
            for (c.children.items) |id| {
                const child = try self.getNode(.{ .id = id });
                try self.drawNode(child);
            }
        },
        .label => |l| {
            const offest: rl.Vector2 = .{
                .x = root.rectangle.x + cast(f32, root.paddings.left),
                .y = root.rectangle.y + cast(f32, root.paddings.top),
            };
            rl.drawText(l.text, @intFromFloat(offest.x), @intFromFloat(offest.y), @intCast(l.font_size), root.foreground.?);
        },
        .scroll_view => |sv| {
            const diff: f32 = cast(f32, sv.viewport.texture.height) - root.rectangle.height;
            // f32 height = -command->rect.height;
            sv.viewport.texture.drawRec(
                .{ .x = 0, .y = diff, .width = root.rectangle.width, .height = -root.rectangle.height },
                .{ .x = root.rectangle.x, .y = root.rectangle.y },
                .white,
            );
        },
    }
}

pub fn draw(self: *Self) !void {
    if (self.current_container == .root) {
        const root = try self.getNode(self.current_container);
        try self.drawNode(root);
    } else unreachable;
}
