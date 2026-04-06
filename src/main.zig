const std = @import("std");
const cast = std.math.lossyCast;

const rl = @import("raylib.zig");
const sqlite3 = @import("sqlite3");

const UI = struct {
    const Direction = enum { down, left };

    const Origin = enum {
        left_top,
        right_bottom,
    };

    var sizes: rl.Vector2 = .{};
    var offset: rl.Vector2 = .{};
    var mouse_position: rl.Vector2 = .{};
    var mouse_down = false;
    var mouse_released = false;
    var direction: Direction = .down;
    var origin: Origin = .left_top;
    var arena: std.heap.ArenaAllocator = undefined;
    var allocator: std.mem.Allocator = undefined;

    fn init() void {
        UI.arena = .init(std.heap.page_allocator);
        UI.allocator = arena.allocator();
    }

    fn frameStart() void {
        UI.mouse_position = rl.Mouse.getPosition();
        UI.mouse_down = rl.Mouse.isButtonDown(.left);
        UI.mouse_released = rl.Mouse.isButtonReleased(.left);
    }

    fn frameEnd() void {
        UI.offset = .{};
        UI.sizes = .{};
        _ = UI.arena.reset(.retain_capacity);
    }

    const StartOptions = struct {
        direction: Direction = .down,
        origin: Origin = .left_top,
    };

    fn start(position: rl.Vector2, options: StartOptions) void {
        UI.offset = position;
        UI.sizes = .{};
        UI.direction = options.direction;
        UI.origin = options.origin;
    }
};

fn startElement(sizes: rl.Vector2, child_gap: f32) struct { rl.Vector2, rl.Rectangle } {
    const offset: rl.Vector2 = switch (UI.direction) {
        .down => .{
            .x = UI.offset.x,
            .y = UI.offset.y + (UI.sizes.y + if (UI.sizes.y > 0) child_gap else 0),
        },
        .left => .{
            .x = UI.offset.x - (UI.sizes.x + if (UI.sizes.x > 0) child_gap else 0),
            .y = UI.offset.y,
        },
    };
    const rect: rl.Rectangle = (switch (UI.origin) {
        .left_top => offset,
        .right_bottom => offset.subtract(sizes),
    }).toRectangleV(sizes);
    return .{ offset, rect };
}

fn endElement(offset: rl.Vector2, sizes: rl.Vector2) void {
    UI.offset = offset;
    UI.sizes = sizes;
}

const Label = struct {
    rect: rl.Rectangle,
    offset: rl.Vector2,
    sizes: rl.Vector2,
    border: f32,
    padding: f32,
    font_size: i32,
    text: [:0]const u8,

    fn start(text: [:0]const u8, width: ?f32, padding: f32, border: f32, font_size: i32, child_gap: f32) Label {
        const sizes: rl.Vector2 = .{
            .x = width orelse (padding + border) * 2 + cast(
                f32,
                rl.measureText(
                    text,
                    font_size,
                ),
            ),
            .y = (padding + border) * 2 + cast(f32, font_size),
        };
        const offset, const rect = startElement(sizes, child_gap);
        return .{
            .rect = rect,
            .sizes = sizes,
            .offset = offset,
            .border = border,
            .text = text,
            .padding = padding,
            .font_size = font_size,
        };
    }

    fn end(self: Label, background: ?rl.Color, foreground: rl.Color) void {
        if (background) |b| self.rect.draw(b);
        if (self.border > 0) self.rect.drawLines(self.border, .black);
        rl.drawText(
            self.text,
            @intFromFloat(self.rect.x + self.padding + self.border),
            @intFromFloat(self.rect.y + self.padding + self.border),
            self.font_size,
            foreground,
        );
        endElement(self.offset, self.sizes);
    }
};

const LabelOptions = struct {
    background: ?rl.Color = null,
    border: f32 = 0,
    width: ?f32 = null,
    padding: f32 = 8,
    child_gap: f32 = 8,
    font_size: i32 = 20,
};

fn label(text: [:0]const u8, options: LabelOptions) void {
    const self = Label.start(
        text,
        options.width,
        options.padding,
        options.border,
        options.font_size,
        options.child_gap,
    );
    self.end(options.background, .black);
}

const ButtonOptions = struct {
    background: rl.Color = .gray,
    border: f32 = 1,
    width: ?f32 = null,
    padding: f32 = 8,
    child_gap: f32 = 8,
    font_size: i32 = 20,
};

fn button(text: [:0]const u8, options: ButtonOptions) bool {
    var self = Label.start(
        text,
        options.width,
        options.padding,
        options.border,
        options.font_size,
        options.child_gap,
    );
    var background: rl.Color = options.background;
    var click = false;
    if (UI.mouse_position.checkCollisionRec(self.rect)) {
        if (UI.mouse_down) {
            background = .dark_gray;
        } else if (UI.mouse_released) {
            click = true;
        } else {
            background = .lime;
        }
    }
    self.end(background, .black);
    return click;
}

const InputOptions = struct {
    background: ?rl.Color = null,
    padding: f32 = 8,
    child_gap: f32 = 8,
    font_size: i32 = 20,
    border: f32 = 1,
};

fn input(
    allocator: std.mem.Allocator,
    placholder: [:0]const u8,
    text: *std.ArrayList(u8),
    width: f32,
    options: InputOptions,
) !void {
    const input_text: [:0]const u8, const foreground: rl.Color = blk: {
        if (text.items.len > 0) {
            break :blk .{ try UI.allocator.dupeSentinel(u8, text.items, 0), .black };
        } else {
            break :blk .{ placholder, .gray };
        }
    };

    const self = Label.start(
        input_text,
        width,
        options.padding,
        options.border,
        options.font_size,
        options.child_gap,
    );

    if (UI.mouse_position.checkCollisionRec(self.rect)) {
        var char = rl.getCharPressed();
        while (char != 0) : (char = rl.getCharPressed()) {
            try text.append(allocator, @intCast(char));
        }

        if (rl.isKeyReleased(.backspace)) {
            _ = text.pop();
        }
    }

    self.end(options.background, foreground);
}

const Tab = struct {
    name: std.ArrayList(u8) = .empty,
    energy: std.ArrayList(u8) = .empty,

    fn deinit(self: *Tab, allocator: std.mem.Allocator) void {
        self.name.deinit(allocator);
        self.energy.deinit(allocator);
    }
};

pub fn main(init: std.process.Init) !void {
    UI.init();

    rl.ConfigFlags.set(.{ .window_resizable = true });

    var width: i32 = 1280;
    var height: i32 = 720;

    rl.Window.init(width, height, "umai");
    defer rl.Window.close();

    rl.setTargetFPS(60);

    // init
    var foods: std.ArrayList([4][:0]const u8) = .empty;
    defer {
        for (foods.items) |food| for (0..food.len) |i| init.gpa.free(food[i]);
        foods.deinit(init.gpa);
    }
    var current_tab: usize = 0;

    var tabs: std.ArrayList(Tab) = .empty;
    defer {
        for (tabs.items) |*tab| {
            tab.deinit(init.gpa);
        }
        tabs.deinit(init.gpa);
    }

    var database, var database_need_update = blk: {
        const path: [:0]const u8 = "database.db";
        const is_file_exists = blk_exist: {
            std.Io.Dir.cwd().access(
                init.io,
                path,
                .{},
            ) catch |err| switch (err) {
                error.FileNotFound => break :blk_exist false,
                else => return err,
            };
            break :blk_exist true;
        };
        var result = sqlite3.Database.init(path);
        if (!is_file_exists) {
            const sql =
                \\CREATE TABLE "food" (
                \\    "id"      INTEGER NOT NULL UNIQUE,
                \\    "created" INTEGER NOT NULL,
                \\    "name"    TEXT NOT NULL,
                \\    "energy"  REAL NOT NULL,
                \\    PRIMARY KEY("id")
                \\) STRICT;
            ;
            result.exec(sql);
        }
        break :blk .{ result, true };
    };
    defer database.deinit();

    while (!rl.Window.shouldClose()) {
        UI.frameStart();
        defer UI.frameEnd();
        // update
        if (rl.Window.isResized()) {
            width = rl.getScreenWidth();
            height = rl.getScreenHeight();
        }
        if (database_need_update) {
            // clear
            for (foods.items) |food| for (0..food.len) |i| init.gpa.free(food[i]);
            foods.clearRetainingCapacity();
            // update
            database_need_update = false;
            try foods.append(init.gpa, .{
                try init.gpa.dupeSentinel(u8, "id", 0),
                try init.gpa.dupeSentinel(u8, "created", 0),
                try init.gpa.dupeSentinel(u8, "name", 0),
                try init.gpa.dupeSentinel(u8, "energy", 0),
            });
            const stmt = database.prepare("SELECT * FROM food");
            defer stmt.deinit();
            var rc = stmt.step();
            while (rc != .done) : (rc = stmt.step()) {
                try foods.append(init.gpa, .{
                    try std.fmt.allocPrintSentinel(
                        init.gpa,
                        "{}",
                        .{stmt.columnInt64(0)},
                        0,
                    ),
                    try std.fmt.allocPrintSentinel(
                        init.gpa,
                        "{}",
                        .{stmt.columnInt64(1)},
                        0,
                    ),
                    try std.fmt.allocPrintSentinel(
                        init.gpa,
                        "{s}",
                        .{stmt.columnText(2)},
                        0,
                    ),
                    try std.fmt.allocPrintSentinel(
                        init.gpa,
                        "{}",
                        .{stmt.columnDouble(3)},
                        0,
                    ),
                });
            }
        }
        // draw
        rl.beginDrawing();
        rl.clearBackground(.light_gray);
        UI.start(.{ .x = 8, .y = 8 }, .{});
        if (button("add a food", .{})) {
            try tabs.append(init.gpa, .{});
        }
        const max_width = 128;
        const padding = 8;
        const margin = 1;
        const font_size = 20;
        const tab_background_rect: rl.Rectangle = blk: {
            const x = padding + max_width;
            const y = UI.sizes.y + padding * 2;
            break :blk .{
                .x = x,
                .y = y,
                .width = cast(f32, width - (x + padding)),
                .height = cast(f32, height) - (y + padding),
            };
        };
        tab_background_rect.draw(.white);
        // main tab is not tab
        for (0..tabs.items.len + 1) |i| {
            const background: rl.Color = if (current_tab == i) .white else .gray;
            if (i == 0) {
                if (button("main", .{
                    .border = 0,
                    .background = background,
                    .width = max_width,
                })) current_tab = i;
            } else {
                const name = tabs.items[i - 1].name.items;
                const text: [:0]const u8 = blk: {
                    var len = name.len;
                    if (len > 0) {
                        var text: []u8 = try UI.allocator.dupeSentinel(u8, name, 0);
                        while (true) {
                            const text_width = rl.measureText(
                                @ptrCast(text),
                                20,
                            ) + padding * 2;
                            if (text_width > max_width) {
                                len -= 1;
                                text[len] = 0;
                            } else break;
                        }
                        break :blk @ptrCast(text);
                    }
                    break :blk "new food";
                };

                if (button(text, .{
                    .border = 0,
                    .background = background,
                    .width = max_width,
                    .child_gap = 0,
                })) current_tab = i;
            }
        }

        if (current_tab == 0) {
            // main tab
            var cells_widths: [4]i32 = .{ 0, 0, 0, 0 };
            for (foods.items) |food| {
                for (food, 0..) |cell, i| {
                    cells_widths[i] = @max(cells_widths[i], rl.measureText(cell, font_size));
                }
            }
            var rect: rl.Rectangle = .{
                .x = tab_background_rect.x + padding,
                .y = tab_background_rect.y + padding,
                .width = margin,
                .height = margin + cast(f32, foods.items.len * (font_size + padding * 2 + margin)),
            };
            for (cells_widths) |w| {
                rect.width += cast(f32, w) + padding * 2 + margin;
            }
            var y: i32 = @intFromFloat(rect.y);
            for (foods.items, 0..) |food, i| {
                var x: i32 = @intFromFloat(rect.x);
                const start_h: rl.Vector2 = .{
                    .x = rect.x,
                    .y = cast(f32, y + font_size) + padding * 2 + margin,
                };
                start_h.drawLine(
                    .{ .y = start_h.y, .x = rect.x + rect.width },
                    .black,
                );
                for (food, 0..) |cell, j| {
                    rl.drawText(
                        cell,
                        x + padding + margin,
                        y + padding + margin,
                        font_size,
                        .black,
                    );
                    if (i == 0) {
                        const start_v: rl.Vector2 = .{
                            .x = cast(f32, x + cells_widths[j]) + (padding + margin) * 2,
                            .y = rect.y,
                        };
                        start_v.drawLine(
                            .{ .x = start_v.x, .y = rect.y + rect.height },
                            .black,
                        );
                    }
                    x += cells_widths[j] + padding * 2 + margin;
                }
                y += font_size + padding * 2 + margin;
            }
            rect.drawLines(1, .black);
        } else {
            // new food tab
            UI.start(tab_background_rect.toVector2().addValue(8), .{});
            label("name", .{});
            label("energy", .{});
            const inputs_width = 128 * 3;
            UI.start(.{
                .x = tab_background_rect.x + 128,
                .y = tab_background_rect.y + 8,
            }, .{});
            try input(
                init.gpa,
                "enter name of food",
                &tabs.items[current_tab - 1].name,
                inputs_width,
                .{},
            );
            try input(
                init.gpa,
                "enter energy value of food",
                &tabs.items[current_tab - 1].energy,
                inputs_width,
                .{},
            );
            UI.start(.{
                .x = tab_background_rect.x + tab_background_rect.width - 8,
                .y = tab_background_rect.y + tab_background_rect.height - 8,
            }, .{ .origin = .right_bottom, .direction = .left });
            if (button("add", .{})) {
                const i = current_tab - 1;
                const name = tabs.items[i].name.items;
                const raw_energy = tabs.items[i].energy.items;
                const energy = try std.fmt.parseFloat(f32, raw_energy);
                const stmt = database.prepare("INSERT INTO food (created, name, energy) VALUES (?, ?, ?)");
                defer stmt.deinit();
                try stmt.bindInt64(1, std.Io.Clock.real.now(init.io).toSeconds());
                try stmt.bindText(2, name, .static);
                try stmt.bindDouble(3, energy);
                std.debug.assert(stmt.step() == .done);
                var tab = tabs.orderedRemove(i);
                tab.deinit(init.gpa);
                current_tab -= 1;
                database_need_update = true;
            }
            if (button("cancel", .{})) {
                var tab = tabs.orderedRemove(current_tab - 1);
                tab.deinit(init.gpa);
                current_tab -= 1;
                database_need_update = true;
            }
        }
        rl.endDrawing();
    }
}
