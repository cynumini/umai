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

    fn fit(node: *Node, ui: *Self, comptime size_type: Node.SizeType, comptime size_str: []const u8) u32 {
        const self = &node.type.container;
        const own_size: u32 = node.calcOwnSize(size_type);
        const main_direction, const other_direction = switch (size_type) {
            .width => .{ Container.Direction.left_to_right, Container.Direction.top_to_bottom },
            .height => .{ Container.Direction.top_to_bottom, Container.Direction.left_to_right },
        };

        for (self.children_ids.items) |id| {
            const child = ui.getNode(id);
            child.fit(ui, size_type);
        }
        if (@field(node, size_str) == .fixed) {
            return @field(node, size_str).fixed;
        }
        switch (self.direction) {
            main_direction => {
                var size = own_size;
                if (self.children_ids.items.len > 0) {
                    size += cast(u32, (self.children_ids.items.len - 1) * self.child_gap);
                }
                for (self.children_ids.items) |id| {
                    const child = ui.getNode(id);
                    size += @intFromFloat(@field(child.rectangle, size_str));
                }
                return size;
            },
            other_direction => {
                var size: u32 = 0;
                for (self.children_ids.items) |id| {
                    const child = ui.getNode(id);
                    size = @max(size, cast(u32, @field(child.rectangle, size_str)));
                }
                return own_size + size;
            },
        }
    }
};

const ScrollView = struct {
    scroll: *i32,
    child_id: ?usize,
    render_texture: *rl.RenderTexture,

    fn fit(node: *Node, ui: *Self, comptime size_type: Node.SizeType, comptime size_str: []const u8) u32 {
        const self = &node.type.scroll_view;
        if (self.child_id) |id| {
            ui.getNode(id).fit(ui, size_type);
        }
        if (@field(node, size_str) == .fixed) {
            return @field(node, size_str).fixed;
        }
        return node.calcOwnSize(size_type);
    }
};

const Label = struct {
    text: [:0]const u8,
    size: u32,
    wrap: bool,

    fn fit(node: *Node, _: *Self, size_type: Node.SizeType, comptime _: []const u8) u32 {
        const self = &node.type.label;
        if (self.wrap) {
            unreachable;
        } else {
            return node.calcOwnSize(size_type) + switch (size_type) {
                .width => rl.measureText(self.text, self.size),
                .height => self.size,
            };
        }
    }
};

const Node = struct {
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

    type: Type,
    id: []const u8,
    width: Size = .fit,
    height: Size = .fit,
    parent: ?usize = null,
    paddings: Paddings,
    border_size: u32,
    background: ?rl.Color,
    foreground: ?rl.Color,
    rectangle: rl.Rectangle = .{},

    fn fit(self: *Node, ui: *Self, size_type: SizeType) void {
        switch (size_type) {
            inline else => |st| {
                const size_str = comptime st.toStr();

                @field(self.rectangle, size_str) = cast(f32, switch (self.type) {
                    .container => Container.fit(self, ui, st, size_str),
                    .scroll_view => ScrollView.fit(self, ui, st, size_str),
                    .label => Label.fit(self, ui, st, size_str),
                });
            },
        }
    }

    const SizeType = enum {
        width,
        height,

        fn toStr(comptime self: SizeType) []const u8 {
            return switch (self) {
                .width => "width",
                .height => "height",
            };
        }
    };

    fn calcOwnSize(self: Node, size_type: SizeType) u32 {
        return switch (size_type) {
            .width => self.paddings.left + self.paddings.right + (self.border_size * 2),
            .height => self.paddings.top + self.paddings.bottom + (self.border_size * 2),
        };
    }
};

const Style = struct {
    paddings: Node.Paddings = .all(8),
    child_gap: u32 = 8,
    border_size: u32 = 0,
    background: ?rl.Color = null,
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
    });
}

fn growWidth(self: *Self, allocator: std.mem.Allocator, root: *Node) !void {
    var remaining_width = root.rectangle.width - cast(f32, root.calcOwnSize(.width));
    var growable = std.ArrayList(*Node).empty;
    defer growable.deinit(allocator);

    const container = if (root.type == .container) &root.type.container else return;

    switch (container.direction) {
        .left_to_right => {
            if (container.children_ids.items.len > 0) {
                remaining_width -= cast(f32, container.child_gap * (container.children_ids.items.len - 1));
            }
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
            for (container.children_ids.items) |id| {
                const child = self.getNode(id);
                if (child.width == .grow) child.rectangle.width = @max(remaining_width, child.rectangle.width);
            }
        },
    }
    for (container.children_ids.items) |id| {
        const child = self.getNode(id);
        if (child.type == .container) try self.growWidth(allocator, child);
    }
}

fn growHeight(self: *Self, allocator: std.mem.Allocator, root: *Node) !void {
    var remaining_height = root.rectangle.height - cast(f32, root.calcOwnSize(.height));
    var growable = std.ArrayList(*Node).empty;
    defer growable.deinit(allocator);

    const container = if (root.type == .container) &root.type.container else return;

    switch (container.direction) {
        .top_to_bottom => {
            if (container.children_ids.items.len > 0) {
                remaining_height -= cast(f32, container.child_gap * (container.children_ids.items.len - 1));
            }
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

fn position(self: *Self, root: *Node) !void {
    var left_offset = root.rectangle.x + cast(f32, root.paddings.right + root.border_size);
    var top_offset = root.rectangle.y + cast(f32, root.paddings.top + root.border_size);

    var remaining_width = root.rectangle.width;
    var remaining_height = root.rectangle.height;

    const container = blk: {
        if (root.type == .container) {
            break :blk &root.type.container;
        } else if (root.type == .scroll_view) {
            if (root.type.scroll_view.child_id) |id| {
                const child = self.getNode(id);
                if (child.type == .container) try self.position(child);
            }
            return;
        } else {
            return;
        }
    };

    for (container.children_ids.items) |id| {
        const child = self.getNode(id);
        remaining_width -= child.rectangle.width;
        remaining_height -= child.rectangle.height;
    }

    switch (container.direction) {
        .left_to_right => {
            if (container.children_ids.items.len > 0) {
                remaining_width -= cast(f32, (container.children_ids.items.len - 1) * container.child_gap);
            }
            if (container.alignment == .center) {
                left_offset += remaining_width / 2;
            }
            for (container.children_ids.items) |id| {
                const child = self.getNode(id);
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
            if (container.children_ids.items.len > 0) {
                remaining_height -= cast(f32, (container.children_ids.items.len - 1) * container.child_gap);
            }
            if (container.alignment == .center) {
                top_offset += remaining_height / 2;
            }
            for (container.children_ids.items) |id| {
                const child = self.getNode(id);
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

    for (container.children_ids.items) |id| {
        const child = self.getNode(id);
        if (child.type == .container or child.type == .scroll_view) try self.position(child);
    }
}

pub fn end(self: *Self, allocator: std.mem.Allocator) !void {
    const node = self.getNode(self.current_container.?);
    if (node.parent == null) {
        const root_root = node;
        node.fit(self, .width);
        try self.growWidth(allocator, root_root);
        // Wrap text
        node.fit(self, .height);
        try self.growHeight(allocator, root_root);
        try self.position(root_root);

        var i: usize = self.nodes.items.len;
        while (i > 0) : (i -= 1) {
            const child = self.getNode(i - 1);
            if (child.type == .scroll_view) {
                var sv = child.type.scroll_view;
                const root = self.getNode(sv.child_id.?);
                const width: i32 = @intFromFloat(child.rectangle.width);
                const height: i32 = @intFromFloat(child.rectangle.height);
                if (width > sv.render_texture.texture.width or height > sv.render_texture.texture.height) {
                    sv.render_texture.deinit();
                    sv.render_texture.* = rl.RenderTexture.init(width, height);
                }
                const temp: rl.Vector2 = .{ .x = root.rectangle.x, .y = root.rectangle.y };
                sv.scroll.* = std.math.clamp(
                    sv.scroll.*,
                    0,
                    cast(i32, root.rectangle.height - child.rectangle.height),
                );
                root.rectangle.x = 0;
                root.rectangle.y = -cast(f32, sv.scroll.*);
                sv.render_texture.begin();
                rl.clearBackground(.white);
                try self.position(root_root);
                try self.drawNode(root);
                sv.render_texture.end();
                root.rectangle.x = temp.x;
                root.rectangle.y = temp.y;
            }
        }
    }
    self.current_container = node.parent;
}

const ScrollViewOptions = struct {
    id: []const u8 = "",
    paddings: ?Node.Paddings = null,
    border_size: ?u32 = null,
    width: Node.Size = .fit,
    height: Node.Size = .fit,
    background: ?rl.Color = null,
    foreground: ?rl.Color = null,
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
        .background = options.background orelse self.style.background,
        .foreground = options.foreground orelse self.style.foreground,
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
};

pub fn label(self: *Self, allocator: std.mem.Allocator, text: [:0]const u8, options: LabelOptions) !void {
    _ = try self.addNode(allocator, .{
        .id = options.id,
        .type = .{ .label = .{
            .text = text,
            .wrap = options.wrap,
            .size = options.font_size orelse self.style.font_size,
        } },
        .paddings = options.paddings orelse self.style.paddings,
        .border_size = options.border_size orelse self.style.border_size,
        .background = options.background orelse self.style.background,
        .foreground = options.foreground orelse self.style.foreground,
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
            for (c.children_ids.items) |id| {
                const child = self.getNode(id);
                try self.drawNode(child);
            }
        },
        .label => |l| {
            const offest: rl.Vector2 = .{
                .x = root.rectangle.x + cast(f32, root.paddings.left),
                .y = root.rectangle.y + cast(f32, root.paddings.top),
            };
            rl.drawText(l.text, @intFromFloat(offest.x), @intFromFloat(offest.y), @intCast(l.size), root.foreground.?);
        },
        .scroll_view => |sv| {
            const diff: f32 = cast(f32, sv.render_texture.texture.height) - root.rectangle.height;
            // f32 height = -command->rect.height;
            sv.render_texture.texture.drawRec(
                .{ .x = 0, .y = diff, .width = root.rectangle.width, .height = -root.rectangle.height },
                .{ .x = root.rectangle.x, .y = root.rectangle.y },
                .white,
            );
        },
    }
}

pub fn draw(self: *Self) !void {
    if (self.current_container == null) {
        const root = self.getNode(0);
        try self.drawNode(root);
    } else unreachable;
}
