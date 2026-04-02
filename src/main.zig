const std = @import("std");
const rl = @import("raylib");
const sqlite3 = @import("sqlite3");

const cast = std.math.lossyCast;

fn isFileExist(io: std.Io, path: [:0]const u8) !bool {
    std.Io.Dir.cwd().access(io, path, .{}) catch |err| switch (err) {
        error.FileNotFound => return false,
        else => return err,
    };
    return true;
}

const Food = struct {
    id: usize,
    created: std.Io.Timestamp,
    name: [:0]const u8,
    energy: f64,

    pub fn init(allocator: std.mem.Allocator, id: usize, created: i96, name: [:0]const u8, energy: f64) !Food {
        return .{
            .id = id,
            .created = .fromNanoseconds(created * std.time.ns_per_s),
            .name = try allocator.dupeSentinel(u8, name, 0),
            .energy = energy,
        };
    }

    pub fn deinit(self: Food, allocator: std.mem.Allocator) void {
        allocator.free(self.name);
    }
};

const Style = struct {
    font_size: i32,
    foreground: rl.Color,
    background: rl.Color,
    inactive_foreground: rl.Color,
    over_color: rl.Color,
    down_color: rl.Color,
    tab_background: rl.Color,
    padding: f32,
    border: f32,
};

pub fn calcTableRect(
    target: rl.Rectangle,
    table_len: usize,
    cells_widths: []f32,
    cells_height: f32,
    border: f32,
) rl.Rectangle {
    var width: f32 = cast(f32, cells_widths.len + 1) * border;
    for (cells_widths) |cell_width| width += cell_width;
    return target.toVector2().toRectangle(
        width,
        cast(f32, table_len) * (cells_height + border) + border,
    );
}

pub fn tab_main(ui: *UI, state: *State, target: rl.Rectangle) !void {
    const style = state.style;
    const len = @typeInfo(Food).@"struct".fields.len;
    var cells_widths: [len]f32 = .{0} ** len;
    const cells_height = cast(f32, style.font_size) + style.padding * 2;
    var table: std.ArrayList([len][:0]const u8) = .empty;

    try table.append(ui.allocator, .{ "id", "created", "name", "energy" });

    for (state.foods.items) |food| {
        try table.append(ui.allocator, .{
            try std.fmt.allocPrintSentinel(
                ui.allocator,
                "{}",
                .{food.id},
                0,
            ),
            try std.fmt.allocPrintSentinel(
                ui.allocator,
                "{}",
                .{food.created.toSeconds()},
                0,
            ),
            food.name,
            try std.fmt.allocPrintSentinel(
                ui.allocator,
                "{}",
                .{food.energy},
                0,
            ),
        });
    }

    for (table.items) |row| {
        for (row, 0..) |str, i| {
            cells_widths[i] = @max(cast(f32, rl.measureText(
                str,
                style.font_size,
            )) + style.padding * 2, cells_widths[i]);
        }
    }

    var rect = calcTableRect(
        target,
        table.items.len,
        &cells_widths,
        cells_height,
        style.border,
    );

    var y: i32 = @intFromFloat(target.y + style.border);
    for (table.items, 0..) |row, i| {
        var x: i32 = @intFromFloat(target.x + style.border);
        for (row, 0..) |line, j| {
            rl.drawText(
                line,
                x + cast(i32, style.padding),
                y + cast(i32, style.padding),
                style.font_size,
                style.foreground,
            );
            if (i == 0) {
                rl.drawLine(
                    x + cast(i32, cells_widths[j] + style.border),
                    @intFromFloat(target.y),
                    x + cast(i32, cells_widths[j] + style.border),
                    @intFromFloat(target.y + rect.height),
                    style.foreground,
                );
            }
            x += @intFromFloat(cells_widths[j] + style.border);
        }
        rl.drawLine(
            @intFromFloat(target.x),
            y + style.font_size + cast(i32, style.padding * 2),
            @intFromFloat(target.x + rect.width),
            y + style.font_size + cast(i32, style.padding * 2),
            style.foreground,
        );
        y += cast(i32, cells_height + state.style.border);
    }
    rect.drawLines(style.border, style.foreground);
}

pub fn tab_new_food(ui: *UI, state: *State, target: rl.Rectangle) void {
    ui.label(state, target, "Name");
    var rect = target;
    rect.x += 100;
    ui.input(state, rect, state.name, "enter food name");
}

pub fn tabs(state: *State, ui: *UI, target: rl.Rectangle) !void {
    const Static = struct {
        var max_width: f32 = 0;
    };
    var offset = target.toVector2();
    {
        const max_width: f32 = Static.max_width;
        Static.max_width = 0;
        for (0..state.tabs_count) |i| {
            const background = if (i == state.current_tab) state.style.tab_background else state.style.background;
            const result = if (i == 0)
                ui.button(
                    state,
                    offset.toRectangle(max_width, 0),
                    "main",
                    .{ .background = background, .border = 0 },
                )
            else
                ui.button(
                    state,
                    offset.toRectangle(max_width, 0),
                    "new food",
                    .{ .background = background, .border = 0 },
                );
            if (result) {
                state.current_tab = i;
            }
            Static.max_width = @max(Static.max_width, ui.rect.width);
            offset.y += ui.rect.height;
        }
    }

    var rect: rl.Rectangle = .{
        .x = target.x + Static.max_width,
        .y = target.y,
        .width = target.width - Static.max_width,
        .height = target.height,
    };
    rect.draw(state.style.tab_background);

    rect = .{
        .x = rect.x + state.style.padding,
        .y = rect.y + state.style.padding,
        .width = rect.width - state.style.padding * 2,
        .height = rect.height - state.style.padding * 2,
    };

    if (state.current_tab == 0) {
        try tab_main(ui, state, rect);
    } else {
        tab_new_food(ui, state, rect);
    }
}

const State = struct {
    tabs_count: usize,
    current_tab: usize,
    foods: std.ArrayList(Food),
    style: Style,
    allocator: std.mem.Allocator,
    name: std.ArrayList(u8),

    fn deinit(self: *State) void {
        for (self.foods.items) |food| {
            food.deinit(self.allocator);
        }
        self.foods.deinit(self.allocator);
    }
};

const UI = struct {
    mouse_position: rl.Vector2,
    mouse_click: bool,
    mouse_down: bool,
    allocator: std.mem.Allocator,
    rect: rl.Rectangle,

    fn label(
        self: *UI,
        state: *State,
        target: rl.Rectangle,
        text: [:0]const u8,
    ) void {
        const style = state.style;
        _ = self;
        rl.drawText(
            text,
            @intFromFloat(target.x),
            @intFromFloat(target.y),
            style.font_size,
            style.foreground,
        );
    }

    fn input(
        self: *UI,
        state: *State,
        target: rl.Rectangle,
        buffer: std.ArrayList(u8),
        placeholder: [:0]const u8,
    ) void {
        _ = self;
        const style = state.style;
        if (buffer.items.len == 0) {
            rl.drawText(
                placeholder,
                @intFromFloat(target.x),
                @intFromFloat(target.y),
                style.font_size,
                style.inactive_foreground,
            );
        }
    }

    fn calcButtonRect(
        target: rl.Rectangle,
        text: [:0]const u8,
        padding_and_border: f32,
        font_size: i32,
    ) rl.Rectangle {
        const width = cast(f32, rl.measureText(text, font_size));
        return .{
            .x = target.x,
            .y = target.y,
            .width = @max(width + padding_and_border * 2, target.width),
            .height = @max(cast(f32, font_size) + padding_and_border * 2, target.height),
        };
    }

    const ButtonOptions = struct {
        background: ?rl.Color = null,
        border: ?f32 = null,
    };

    fn button(
        self: *UI,
        state: *State,
        target: rl.Rectangle,
        text: [:0]const u8,
        options: ButtonOptions,
    ) bool {
        const style = state.style;
        const border = options.border orelse style.border;
        const padding_and_border = style.padding + border;
        self.rect = calcButtonRect(
            target,
            text,
            padding_and_border,
            style.font_size,
        );
        var background = options.background orelse style.background;
        const mouse_click = blk: {
            if (self.mouse_position.checkCollisionRec(self.rect)) {
                if (self.mouse_click) break :blk true;
                background = if (self.mouse_down) style.down_color else style.over_color;
            }
            break :blk false;
        };
        self.rect.draw(background);
        if (border > 0) {
            self.rect.drawLines(border, style.foreground);
        }
        rl.drawText(
            text,
            cast(i32, self.rect.x + padding_and_border),
            cast(i32, self.rect.y + padding_and_border),
            style.font_size,
            style.foreground,
        );
        return mouse_click;
    }
};

fn main_ui(state: *State, ui: *UI, target: rl.Rectangle) !void {
    const panel_height = blk: {
        if (ui.button(state, target.toVector2().toRectangleZero(), "add a food", .{})) {
            state.tabs_count += 1;
        }
        break :blk ui.rect.height;
    } + state.style.padding;
    try tabs(state, ui, .{
        .x = target.x,
        .y = target.y + panel_height,
        .width = target.width,
        .height = target.height - panel_height,
    });
}

pub fn main(init: std.process.Init) !void {
    var width: u32 = 1280;
    var height: u32 = 720;

    var database = blk: {
        const path: [:0]const u8 = "database.db";
        const exist = try isFileExist(init.io, path);
        var result_database = sqlite3.Database.init(path);

        if (!exist) {
            const sql =
                \\CREATE TABLE "food" (
                \\    "id"      INTEGER NOT NULL UNIQUE,
                \\    "created" INTEGER NOT NULL,
                \\    "name"    TEXT NOT NULL,
                \\    "energy"  REAL NOT NULL,
                \\    PRIMARY KEY("id")
                \\) STRICT;
            ;
            result_database.exec(sql);
        }
        break :blk result_database;
    };
    defer database.deinit();
    var database_need_update = true;

    var state = State{
        .current_tab = 0,
        .tabs_count = 1,
        .foods = .empty,
        .allocator = init.gpa,
        .name = .empty,
        .style = Style{
            .font_size = 20,
            .foreground = .black,
            .inactive_foreground = .gray,
            .background = .gray,
            .tab_background = .white,
            .padding = 8,
            .border = 1,
            .over_color = .lime,
            .down_color = .dark_gray,
        },
    };
    defer state.deinit();

    //var scroll: i32 = 0;
    //var tab_index: usize = 0;

    rl.ConfigFlags.set(.{ .window_resizable = true });
    const window = rl.Window.init(width, height, "umai");
    defer window.deinit();

    rl.setTargetFPS(60);

    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();

    var v = rl.RenderTexture.init(0, 0);
    defer v.deinit();

    while (!window.shouldClose()) {
        defer _ = arena.reset(.retain_capacity);

        // update
        var ui = UI{
            .mouse_click = rl.isMouseButtonPressed(.left),
            .mouse_down = rl.isMouseButtonDown(.left),
            .mouse_position = rl.getMousePosition(),
            .allocator = arena.allocator(),
            .rect = .{},
        };
        if (window.isResized()) {
            width = rl.getScreenWidth();
            height = rl.getScreenHeight();
        }

        if (database_need_update) {
            // clear table
            for (state.foods.items) |food| food.deinit(init.gpa);
            state.foods.clearRetainingCapacity();
            // end clear table
            database_need_update = false;
            const stmt = database.prepare("SELECT * FROM food");
            defer stmt.deinit();
            var rc = stmt.step();
            while (rc != .done) : (rc = stmt.step()) {
                try state.foods.append(init.gpa, try Food.init(
                    init.gpa,
                    @intCast(stmt.columnInt64(0)),
                    stmt.columnInt64(1),
                    stmt.columnText(2),
                    stmt.columnDouble(3),
                ));
            }
        }

        // draw
        rl.beginDrawing();
        rl.clearBackground(.light_gray);
        try main_ui(&state, &ui, .{
            .x = state.style.padding,
            .y = state.style.padding,
            .width = cast(f32, width) - state.style.padding * 2,
            .height = cast(f32, height) - state.style.padding * 2,
        });
        rl.endDrawing();
    }
}
