const std = @import("std");
const rl = @import("raylib");

const assert = std.debug.assert;

var arena: ?std.heap.ArenaAllocator = null;
var allocator: ?std.mem.Allocator = null;
var current_container_id: ?usize = null;
var current_id: ?usize = null;

const Container = struct {
    const Direction = enum { left_to_right, top_to_bottom };
    const Alignment = enum { left_top, left_center, left_bottom, center_top, center, center_bottom, right_top, right_center, right_bottom };
    direction: Direction,
    alignment: Alignment,
    child_gap: u32,
    children: std.ArrayList(usize) = .empty,
};

const ScrollView = struct {
    scroll: *u32,
    child: ?usize,
};

const Label = struct {
    text: [:0]const u8,
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

    type: Type,
    parent_id: ?usize = null,
    paddings: Sides,
    border_size: u32,
    width: Size,
    height: Size,
    background: ?rl.Color,
    foreground: ?rl.Color,

    fn get_parent(self: *const Node) ?*Node {
        const id = self.parent_id orelse return null;
        return nodes.items[id];
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

var nodes: std.ArrayList(Node) = undefined;

const RootContainerOptions = struct {
    direction: Container.Direction = .left_to_right,
    alignment: Container.Alignment = .left_top,
    background: ?rl.Color = null,
    foreground: ?rl.Color = null,
    paddings: ?Node.Sides = null,
    child_gap: ?u32 = null,
    border_size: ?u32 = null,
};

pub fn begin(width: u32, height: u32, options: RootContainerOptions) !void {
    if (arena == null) {
        arena = .init(std.heap.page_allocator);
        allocator = arena.?.allocator();
    }
    nodes = .empty;
    current_id = null;
    try container_begin(.{
        .direction = options.direction,
        .alignment = options.alignment,
        .paddings = options.paddings,
        .child_gap = options.child_gap,
        .border_size = options.border_size,
        .width = .{ .fixed = width },
        .height = .{ .fixed = height },
        .background = options.background,
        .foreground = options.foreground,
    });
}

pub fn end() void {
    container_end();
    assert(arena.?.reset(.retain_capacity));
}

fn get_current_container() *Node {
    var container = &nodes.items[current_container_id.?];
    assert(container.type == .container or container.type == .scroll_view);
    return container;
}

fn add_node(node: Node) !usize {
    var mut_node = node;
    if (current_id) |*id| {
        var container = get_current_container();
        mut_node.parent_id = current_container_id;
        id.* += 1;
        switch (container.type) {
            .container => |*c| try c.children.append(allocator.?, id.*),
            .scroll_view => |*sv| sv.child = id.*,
            else => unreachable,
        }
    } else current_id = 0;
    try nodes.append(allocator.?, mut_node);
    return current_id.?;
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

pub fn container_begin(options: ContainerOptions) !void {
    current_container_id = try add_node(.{
        .type = .{ .container = .{
            .direction = options.direction,
            .alignment = options.alignment,
            .child_gap = options.child_gap orelse style.child_gap,
        } },
        .paddings = options.paddings orelse style.paddings,
        .border_size = options.border_size orelse style.border_size,
        .width = options.width,
        .height = options.height,
        .background = options.background orelse style.background,
        .foreground = options.foreground orelse style.foreground,
    });
}

pub fn container_end() void {
    current_container_id = get_current_container().parent_id;
}

const ScrollViewOptions = struct {
    paddings: ?Node.Sides = null,
    border_size: ?u32 = null,
    width: Node.Size = .fit,
    height: Node.Size = .fit,
    background: ?rl.Color = null,
    foreground: ?rl.Color = null,
};

pub fn scroll_view_begin(scroll: *u32, options: ScrollViewOptions) !void {
    const wheel_mode = rl.get_mouse_wheel_move();
    if (wheel_mode > 0) {
        scroll.* += 128; // TODO: adjut value or use get_mouse_wheel_move directly
    } else if (wheel_mode > 0) {
        scroll.* -= 128; // TODO: adjut value or use get_mouse_wheel_move directly
    }
    current_container_id = try add_node(.{
        .type = .{ .scroll_view = .{ .scroll = scroll, .child = null } },
        .paddings = options.paddings orelse style.paddings,
        .border_size = options.border_size orelse style.border_size,
        .width = options.width,
        .height = options.height,
        .background = options.background orelse style.background,
        .foreground = options.foreground orelse style.foreground,
    });
}

pub fn scroll_view_end() void {
    container_end();
}

const LabelOptions = struct {
    paddings: ?Node.Sides = null,
    border_size: ?u32 = null,
    font_size: ?u32 = null,
    background: ?rl.Color = null,
    foreground: ?rl.Color = null,
};

pub fn label(text: [:0]const u8, options: LabelOptions) !void {
    const height = options.font_size orelse style.font_size;
    const width = rl.measure_text(text, height);

    _ = try add_node(.{
        .type = .{ .label = .{ .text = text } },
        .paddings = options.paddings orelse style.paddings,
        .border_size = options.border_size orelse style.border_size,
        .width = .{ .fixed = width },
        .height = .{ .fixed = options.font_size orelse style.font_size },
        .background = options.background orelse style.background,
        .foreground = options.foreground orelse style.foreground,
    });
}

pub fn label_fmt(comptime fmt: []const u8, args: anytype, options: LabelOptions) !void {
    try label(try std.fmt.allocPrintSentinel(allocator.?, fmt, args, 0), options);
}

pub fn draw() void {
    // TODO: continue here
}
