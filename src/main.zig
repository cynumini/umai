const std = @import("std");
const cast = std.math.lossyCast;

const rl = @import("raylib.zig");
const sqlite3 = @import("sqlite3");

const UI = struct {
    var rect: rl.Rectangle = .{ .x = 8 };
    var mouse_position: rl.Vector2 = .{};
    var mouse_down = false;
    var mouse_released = false;

    fn update() void {
        UI.rect = .{ .x = 8 };
        UI.mouse_position = rl.Mouse.getPosition();
        UI.mouse_down = rl.Mouse.isButtonDown(.left);
        UI.mouse_released = rl.Mouse.isButtonReleased(.left);
    }
};

const ButtonOptions = struct {
    background: rl.Color = .gray,
    border: f32 = 1,
    width: ?f32 = null,
    padding: f32 = 8,
    child_gap: f32 = 8,
};

pub fn button(text: [:0]const u8, options: ButtonOptions) bool {
    const offset = rl.Vector2{
        .x = UI.rect.x,
        .y = UI.rect.y + UI.rect.height + options.child_gap,
    };
    const width = blk: {
        if (options.width) |width| {
            break :blk width;
        } else {
            break :blk cast(
                f32,
                rl.measureText(text, 20),
            ) + (options.padding + options.border) * 2;
        }
    };
    const rect = rl.Rectangle{
        .x = offset.x,
        .y = offset.y,
        .width = width,
        .height = 20 + (options.padding + options.border) * 2,
    };

    var background: rl.Color = options.background;

    var click = false;

    if (UI.mouse_position.checkCollisionRec(rect)) {
        if (UI.mouse_down) {
            background = .dark_gray;
        } else if (UI.mouse_released) {
            click = true;
        } else {
            background = .lime;
        }
    }

    rect.draw(background);
    if (options.border > 0) {
        rect.drawLines(options.border, .black);
    }
    rl.drawText(
        text,
        @intFromFloat(offset.x + 8 + options.border),
        @intFromFloat(offset.y + 8 + options.border),
        20,
        .black,
    );
    UI.rect = rect;
    return click;
}

pub fn main(init: std.process.Init) !void {
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
    var tabs_count: usize = 1;
    var current_tab: usize = 0;

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
        // update
        if (rl.Window.isResized()) {
            width = rl.getScreenWidth();
            height = rl.getScreenHeight();
        }
        UI.update();
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
        if (button("add a food", .{})) {
            tabs_count += 1;
        }
        const max_width = 128;
        const padding = 8;
        const margin = 1;
        const font_size = 20;
        const tab_background_rect: rl.Rectangle = blk: {
            const x = padding + max_width;
            const y = UI.rect.height + padding * 2;
            break :blk .{
                .x = x,
                .y = y,
                .width = cast(f32, width - (x + padding)),
                .height = cast(f32, height) - (y + padding),
            };
        };
        tab_background_rect.draw(.white);
        for (0..tabs_count) |i| {
            const background: rl.Color = if (current_tab == i) .white else .gray;
            if (i == 0) {
                if (button("main", .{
                    .border = 0,
                    .background = background,
                    .width = max_width,
                })) current_tab = i;
            } else {
                if (button("new food", .{
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
            const rect: rl.Rectangle = .{
                .x = tab_background_rect.x + padding,
                .y = tab_background_rect.y + padding,
            };
            var y: f32 = rect.y;
            inline for (&.{ "name:", "energy:" }) |field| {
                rl.drawText(
                    field,
                    @intFromFloat(rect.x),
                    @intFromFloat(y + padding + margin),
                    font_size,
                    .black,
                );
                (rl.Rectangle{
                    .x = rect.x + 100,
                    .y = y,
                    .width = 256,
                    .height = font_size + (padding + margin) * 2,
                }).drawLines(1, .black);

                y += font_size + (padding + margin) * 2 + padding;
            }
            UI.rect.x = tab_background_rect.x + tab_background_rect.width - 100;
            _ = button("add", .{});
            _ = button("cancel", .{});
        }
        rl.endDrawing();
    }
}
