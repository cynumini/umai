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
    over_color: rl.Color,
    down_color: rl.Color,
    tab_background: rl.Color,
    padding: f32,
    border: f32,
};

pub fn main_tab(state: *State, frame_state: *UI, target: rl.Rectangle) void {
    _ = frame_state;
    var y = target.y;
    // TODO: calc widhts for food struct
    // const len = @typeInfo(Food).@"enum".fields.len;
    for (state.table.items) |food| {
        rl.drawText(food.name, cast(i32, target.x), cast(i32, y), 20, state.style.foreground);
        y += 20;
    }
}

pub fn tabs(state: *State, ui: *UI, target: rl.Rectangle) void {
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

    const rect: rl.Rectangle = .{
        .x = target.x + Static.max_width,
        .y = target.y,
        .width = target.width - Static.max_width,
        .height = target.height,
    };
    rect.draw(state.style.tab_background);

    if (state.current_tab == 0) {
        main_tab(state, ui, .{
            .x = rect.x + state.style.padding,
            .y = rect.y + state.style.padding,
            .width = rect.width - state.style.padding * 2,
            .height = rect.height - state.style.padding * 2,
        });
    }
}

const State = struct {
    tabs_count: usize,
    current_tab: usize,
    table: std.ArrayList(Food),
    style: Style,
};

const UI = struct {
    mouse_position: rl.Vector2,
    mouse_click: bool,
    mouse_down: bool,
    allocator: std.mem.Allocator,
    rect: rl.Rectangle,

    const ButtonOptions = struct {
        background: ?rl.Color = null,
        border: ?f32 = null,
    };

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

    fn button(self: *UI, state: *State, target: rl.Rectangle, text: [:0]const u8, options: ButtonOptions) bool {
        const style = state.style;
        const padding_and_border = style.padding + (options.border orelse style.border);
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

fn main_ui(state: *State, ui: *UI, target: rl.Rectangle) void {
    const panel_height = blk: {
        if (ui.button(state, target.toVector2().toRectangleZero(), "add a food", .{})) {
            state.tabs_count += 1;
        }
        break :blk ui.rect.height;
    } + state.style.padding;
    tabs(state, ui, .{
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
        .table = .empty,
        .style = Style{
            .font_size = 20,
            .foreground = .black,
            .background = .gray,
            .tab_background = .white,
            .padding = 8,
            .border = 1,
            .over_color = .lime,
            .down_color = .dark_gray,
        },
    };

    defer {
        for (state.table.items) |food| {
            food.deinit(init.gpa);
        }
        state.table.deinit(init.gpa);
    }

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
            for (state.table.items) |food| food.deinit(init.gpa);
            state.table.clearRetainingCapacity();
            // end clear table
            database_need_update = false;
            const stmt = database.prepare("SELECT * FROM food");
            defer stmt.deinit();
            var rc = stmt.step();
            while (rc != .done) : (rc = stmt.step()) {
                try state.table.append(init.gpa, try Food.init(
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
        main_ui(&state, &ui, .{
            .x = state.style.padding,
            .y = state.style.padding,
            .width = cast(f32, width) - state.style.padding * 2,
            .height = cast(f32, height) - state.style.padding * 2,
        });
        rl.endDrawing();
    }
}
