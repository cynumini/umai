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

const Button = struct {
    text: [:0]const u8,
    rect: rl.Rectangle,
    background: rl.Color,
    foreground: rl.Color,
    padding: f32,
    border: f32,
    mouse_click: bool,
    font_size: i32,

    const Options = struct {
        background: ?rl.Color = null,
        border: ?f32 = null,
    };

    fn init(state: *State, frame_state: *FrameState, target: rl.Rectangle, text: [:0]const u8, options: Options) Button {
        const style = state.style;
        const border = options.border orelse style.border;
        const width: f32 = @floatFromInt(rl.measureText(text, style.font_size));
        const rect = rl.Rectangle{
            .x = target.x,
            .y = target.y,
            .width = @max(width + (style.padding + border) * 2, target.width),
            .height = @max(cast(f32, style.font_size) + (style.padding + border) * 2, target.height),
        };
        var mouse_click = false;
        var background = options.background orelse style.background;
        if (frame_state.mouse_position.checkCollisionRec(rect)) {
            if (frame_state.mouse_click) mouse_click = true;
            background = if (frame_state.mouse_down) style.down_color else style.over_color;
        }
        return .{
            .text = text,
            .rect = rect,
            .background = background,
            .foreground = style.foreground,
            .border = border,
            .mouse_click = mouse_click,
            .padding = style.padding,
            .font_size = style.font_size,
        };
    }

    fn click(self: Button) bool {
        self.rect.draw(self.background);
        if (self.border > 0) {
            self.rect.drawLines(self.border, self.foreground);
        }
        rl.drawText(
            self.text,
            cast(i32, self.rect.x + self.padding + self.border),
            cast(i32, self.rect.y + self.padding + self.border),
            self.font_size,
            self.foreground,
        );
        return self.mouse_click;
    }
};

pub fn main_tab(state: *State, frame_state: *FrameState, target: rl.Rectangle) void {
    _ = frame_state;
    var y = target.y;
    for (state.table.items) |food| {
        rl.drawText(food.name, cast(i32, target.x), cast(i32, y), 20, state.style.foreground);
        y += 20;
    }
}

pub fn tabs(state: *State, frame_state: *FrameState, target: rl.Rectangle) void {
    const Static = struct {
        var max_width: f32 = 0;
    };
    var offset = target.toVector2();
    {
        const max_width: f32 = Static.max_width;
        Static.max_width = 0;
        for (0..state.tabs_count) |i| {
            const background = if (i == state.current_tab) state.style.tab_background else state.style.background;
            const button = if (i == 0)
                Button.init(
                    state,
                    frame_state,
                    offset.toRectangle(max_width, 0),
                    "main",
                    .{ .background = background, .border = 0 },
                )
            else
                Button.init(
                    state,
                    frame_state,
                    offset.toRectangle(max_width, 0),
                    "new food",
                    .{ .background = background, .border = 0 },
                );
            if (button.click()) {
                state.current_tab = i;
            }
            Static.max_width = @max(Static.max_width, button.rect.width);
            offset.y += button.rect.height;
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
        main_tab(state, frame_state, .{
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

const FrameState = struct {
    mouse_position: rl.Vector2,
    mouse_click: bool,
    mouse_down: bool,
    allocator: std.mem.Allocator,
};

fn ui(state: *State, frame_state: *FrameState, target: rl.Rectangle) void {
    const panel_height = blk: {
        const button = Button.init(
            state,
            frame_state,
            target.toVector2().toRectangleZero(),
            "add a food",
            .{},
        );
        if (button.click()) {
            state.tabs_count += 1;
        }
        break :blk button.rect.height;
    } + state.style.padding;
    tabs(state, frame_state, .{
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
        var frame_state = FrameState{
            .mouse_click = rl.isMouseButtonPressed(.left),
            .mouse_down = rl.isMouseButtonDown(.left),
            .mouse_position = rl.getMousePosition(),
            .allocator = arena.allocator(),
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
        ui(&state, &frame_state, .{
            .x = state.style.padding,
            .y = state.style.padding,
            .width = cast(f32, width) - state.style.padding * 2,
            .height = cast(f32, height) - state.style.padding * 2,
        });
        rl.endDrawing();
    }
}
