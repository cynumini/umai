const std = @import("std");
const cast = std.math.lossyCast;

const rl = @import("raylib.zig");
const sqlite3 = @import("sqlite3");

const ui = @import("ui.zig");
const UI = ui.UI;
const Style = ui.Style;

const State = struct {
    const Database = struct {
        db: sqlite3.Database,
        need_update: bool = true,

        fn init(io: std.Io) !Database {
            const path: [:0]const u8 = "database.db";
            const is_file_exists = blk: {
                std.Io.Dir.cwd().access(
                    io,
                    path,
                    .{},
                ) catch |err| switch (err) {
                    error.FileNotFound => break :blk false,
                    else => return err,
                };
                break :blk true;
            };
            var db = sqlite3.Database.init(path);
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
                db.exec(sql);
            }
            return .{ .db = db };
        }

        fn deinit(self: *Database) void {
            self.db.deinit();
        }
    };
    const Edit = struct {
        name: std.ArrayList(u8) = .empty,
        energy: std.ArrayList(u8) = .empty,

        fn deinit(self: *Edit, allocator: std.mem.Allocator) void {
            self.name.deinit(allocator);
            self.energy.deinit(allocator);
        }
    };

    edit: Edit,
    database: Database,

    fn init(io: std.Io) !State {
        return .{ .edit = .{}, .database = try .init(io) };
    }

    fn deinit(self: *State, allocator: std.mem.Allocator) void {
        self.edit.deinit(allocator);
    }
};

const Tab = struct {
    name: std.ArrayList(u8) = .empty,
    energy: std.ArrayList(u8) = .empty,

    fn deinit(self: *Tab, allocator: std.mem.Allocator) void {
        self.name.deinit(allocator);
        self.energy.deinit(allocator);
    }
};

fn uiEditPanel(allocator: std.mem.Allocator, rect: rl.Rectangle, state: *State) !void {
    rect.draw(.light_gray);
    UI.start(rect.toVector2().addValue(Style.padding), .{});
    ui.label("name", .{});
    if (try ui.input(
        allocator,
        "",
        &state.edit.name,
        rect.width - Style.padding,
        .{},
    )) {}
    ui.label("energy", .{});
    if (try ui.input(
        allocator,
        "",
        &state.edit.energy,
        rect.width - Style.padding,
        .{},
    )) {}
}

pub fn main(init: std.process.Init) !void {
    var state: State = try .init(init.io);
    defer state.deinit(init.gpa);

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
    var table_current_row: ?usize = null;

    var tabs: std.ArrayList(Tab) = .empty;
    defer {
        for (tabs.items) |*tab| {
            tab.deinit(init.gpa);
        }
        tabs.deinit(init.gpa);
    }

    while (!rl.Window.shouldClose()) {
        ui.frameStart();
        defer ui.frameEnd();
        // update
        if (rl.Window.isResized()) {
            width = rl.getScreenWidth();
            height = rl.getScreenHeight();
        }
        if (state.database.need_update) {
            // clear
            for (foods.items) |food| for (0..food.len) |i| init.gpa.free(food[i]);
            foods.clearRetainingCapacity();
            // update
            state.database.need_update = false;
            try foods.append(init.gpa, .{
                try init.gpa.dupeSentinel(u8, "id", 0),
                try init.gpa.dupeSentinel(u8, "created", 0),
                try init.gpa.dupeSentinel(u8, "name", 0),
                try init.gpa.dupeSentinel(u8, "energy", 0),
            });
            const stmt = state.database.db.prepare("SELECT * FROM food");
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
        UI.start(.{ .x = Style.padding, .y = Style.padding }, .{});
        if (ui.button("add a food", .{})) {
            try tabs.append(init.gpa, .{});
        }
        const max_width = 128;
        const margin = 1;
        const font_size = 20;
        const tab_background_rect: rl.Rectangle = blk: {
            const x = Style.padding + max_width;
            const y = UI.sizes.y + Style.padding * 2;
            break :blk .{
                .x = x,
                .y = y,
                .width = cast(f32, width - (x + Style.padding)),
                .height = cast(f32, height) - (y + Style.padding),
            };
        };
        tab_background_rect.draw(.white);
        // main tab is not tab
        for (0..tabs.items.len + 1) |i| {
            const background: rl.Color = if (current_tab == i) .white else .gray;
            if (i == 0) {
                if (ui.button("main", .{
                    .border_color = background,
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
                            ) + Style.padding * 2;
                            if (text_width > max_width) {
                                len -= 1;
                                text[len] = 0;
                            } else break;
                        }
                        break :blk @ptrCast(text);
                    }
                    break :blk "new food";
                };

                if (ui.button(text, .{
                    .border_color = background,
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
                .x = tab_background_rect.x + Style.padding,
                .y = tab_background_rect.y + Style.padding,
                .width = margin,
                .height = margin + cast(f32, foods.items.len * (font_size + Style.padding * 2 + margin)),
            };
            for (cells_widths) |w| {
                rect.width += cast(f32, w) + Style.padding * 2 + margin;
            }

            const row_height = font_size + Style.padding * 2 + margin;

            if (table_current_row) |current| {
                const selected_background = rl.Rectangle{
                    .x = rect.x,
                    .y = rect.y + row_height + (row_height * cast(f32, current)),
                    .width = rect.width,
                    .height = row_height,
                };
                selected_background.draw(.blue);
            }

            var y: i32 = @intFromFloat(rect.y);
            for (foods.items, 0..) |food, i| {
                var x: i32 = @intFromFloat(rect.x);
                const start_h: rl.Vector2 = .{
                    .x = rect.x,
                    .y = cast(f32, y + font_size) + Style.padding * 2 + margin,
                };
                start_h.drawLine(
                    .{ .y = start_h.y, .x = rect.x + rect.width },
                    .black,
                );
                for (food, 0..) |cell, j| {
                    rl.drawText(
                        cell,
                        x + Style.padding + margin,
                        y + Style.padding + margin,
                        font_size,
                        .black,
                    );
                    if (i == 0) {
                        const start_v: rl.Vector2 = .{
                            .x = cast(f32, x + cells_widths[j]) + (Style.padding + margin) * 2,
                            .y = rect.y,
                        };
                        start_v.drawLine(
                            .{ .x = start_v.x, .y = rect.y + rect.height },
                            .black,
                        );
                    }
                    x += cells_widths[j] + Style.padding * 2 + margin;
                }
                y += row_height;
            }
            rect.drawLines(1, .black);

            if (ui.mouse_position.checkCollisionRec(rect)) {
                if (ui.mouse_released) {
                    const relative = ui.mouse_position.subtract(rect.toVector2());
                    const index: usize = @intFromFloat(relative.y / row_height);
                    if (index > 0) {
                        table_current_row = index - 1;
                        state.edit.name.clearRetainingCapacity();
                        try state.edit.name.appendSlice(
                            init.gpa,
                            foods.items[index][2],
                        );
                        state.edit.energy.clearRetainingCapacity();
                        try state.edit.energy.appendSlice(
                            init.gpa,
                            foods.items[index][3],
                        );
                    }
                }
            }
            try uiEditPanel(init.gpa, .{
                .x = tab_background_rect.x + tab_background_rect.width / 2,
                .y = tab_background_rect.y,
                .width = tab_background_rect.width / 2,
                .height = tab_background_rect.height,
            }, &state);
        } else {
            // new food tab
            UI.start(tab_background_rect.toVector2().addValue(8), .{});
            ui.label("name", .{});
            ui.label("energy", .{});
            const inputs_width = 128 * 3;
            UI.start(.{
                .x = tab_background_rect.x + 128,
                .y = tab_background_rect.y + Style.padding,
            }, .{});
            _ = try ui.input(
                init.gpa,
                "enter name of food",
                &tabs.items[current_tab - 1].name,
                inputs_width,
                .{},
            );
            _ = try ui.input(
                init.gpa,
                "enter energy value of food",
                &tabs.items[current_tab - 1].energy,
                inputs_width,
                .{},
            );
            UI.start(.{
                .x = tab_background_rect.x + tab_background_rect.width - Style.padding,
                .y = tab_background_rect.y + tab_background_rect.height - Style.padding,
            }, .{ .origin = .right_bottom, .direction = .left });
            if (ui.button("add", .{})) {
                const i = current_tab - 1;
                const name = tabs.items[i].name.items;
                const raw_energy = tabs.items[i].energy.items;
                const energy = try std.fmt.parseFloat(f32, raw_energy);
                const stmt = state.database.db.prepare("INSERT INTO food (created, name, energy) VALUES (?, ?, ?)");
                defer stmt.deinit();
                try stmt.bindInt64(1, std.Io.Clock.real.now(init.io).toSeconds());
                try stmt.bindText(2, name, .static);
                try stmt.bindDouble(3, energy);
                std.debug.assert(stmt.step() == .done);
                var tab = tabs.orderedRemove(i);
                tab.deinit(init.gpa);
                current_tab -= 1;
                state.database.need_update = true;
            }
            if (ui.button("cancel", .{})) {
                var tab = tabs.orderedRemove(current_tab - 1);
                tab.deinit(init.gpa);
                current_tab -= 1;
                state.database.need_update = true;
            }
        }
        rl.endDrawing();
    }
}
