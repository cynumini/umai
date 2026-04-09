const std = @import("std");
const cast = std.math.lossyCast;

const rl = @import("raylib");
const sqlite3 = @import("sqlite3");

const State = @import("state.zig");
const table = @import("table.zig");
const ui = @import("ui.zig");
const UI = ui.UI;
const Style = ui.Style;

fn uiTabs(rect: rl.Rectangle, state: *State) !void {
    UI.start(rect.toVector2(), .{});
    for (state.tabs.items, 0..) |*tab, i| {
        const background: rl.Color = if (state.current_tab == i) Style.background else Style.inactive;
        const button_options = ui.ButtonOptions{
            .background = background,
            .width = rect.width,
            .border = 0,
            .child_gap = 0,
        };
        switch (tab.*) {
            .main => {
                if (ui.button("main", button_options)) state.current_tab = i;
            },
            .new_food => |*nf| {
                const name = try nf.name.get(ui.allocator);
                const text: []const u8 = blk: {
                    if (nf.name.data.items.len > 0) {
                        for (1..nf.name.data.items.len + 1) |j| {
                            const width: f32 = @floatFromInt(
                                rl.measureText(i32, nf.name.data.items[0..j], Style.font_size),
                            );
                            if (width > (rect.width - Style.padding * 2)) break :blk name[0 .. j - 1];
                        } else {
                            break :blk name;
                        }
                    }
                    break :blk "new food";
                };
                if (ui.button(text, button_options)) state.current_tab = i;
            },
        }
    }
}

fn uiTabMain(allocator: std.mem.Allocator, rect: rl.Rectangle, state: *State) !void {
    const old_index = state.table_current_row;
    try table.table(.{
        .x = rect.x,
        .y = rect.y,
        .width = rect.width / 2,
        .height = rect.height,
    }, state);

    if (old_index != state.table_current_row) {
        if (state.table_current_row) |i| {
            try state.edit.name.set(allocator, state.foods_str.items[i + 1][2]);
            try state.edit.energy.set(allocator, state.foods_str.items[i + 1][3]);
        }
    }

    if (state.table_current_row != null) {
        try uiEditPanel(allocator, .{
            .x = rect.x + rect.width / 2,
            .y = rect.y,
            .width = rect.width / 2,
            .height = rect.height,
        }, state);
    }
}

fn uiTabNewFood(
    allocator: std.mem.Allocator,
    io: std.Io,
    rect: rl.Rectangle,
    state: *State,
    tab: *State.Edit,
) !void {
    // new food tab
    UI.start(.{ .x = rect.x, .y = rect.y + Style.padding }, .{});
    var max_width: f32 = 0;
    ui.label("name", .{});
    max_width = @max(max_width, ui.last_rect.width);
    ui.label("energy", .{});
    max_width = @max(max_width, ui.last_rect.width);
    const inputs_width = rect.width - max_width - Style.padding;
    UI.start(.{ .x = rect.x + max_width, .y = rect.y + Style.padding }, .{});
    _ = try ui.input(allocator, &tab.name, inputs_width, .{});
    _ = try ui.input(allocator, &tab.energy, inputs_width, .{});
    UI.start(.{
        .x = rect.x + rect.width - Style.padding,
        .y = rect.y + rect.height - Style.padding,
    }, .{ .origin = .right_bottom, .direction = .left });
    if (ui.button("add", .{})) {
        const name = try tab.name.get(ui.allocator);
        const raw_energy = try tab.energy.get(ui.allocator);
        const energy = try std.fmt.parseFloat(f32, raw_energy);
        std.debug.print("{}\n", .{energy});
        const stmt = state.database.db.prepare("INSERT INTO food (created, name, energy) VALUES (?, ?, ?)");
        defer stmt.deinit();
        try stmt.bindInt64(1, std.Io.Clock.real.now(io).toSeconds());
        try stmt.bindText(2, name, .static);
        try stmt.bindDouble(3, energy);
        std.debug.assert(stmt.step() == .done);
        var removed_tab = state.tabs.orderedRemove(state.current_tab);
        removed_tab.deinit(allocator);
        state.current_tab -= 1;
        state.database.need_update = true;
    }
    if (ui.button("cancel", .{})) {
        _ = state.tabs.orderedRemove(state.current_tab);
        tab.deinit(allocator);
        state.current_tab -= 1;
        state.database.need_update = true;
    }
}

fn uiTab(allocator: std.mem.Allocator, io: std.Io, rect: rl.Rectangle, state: *State) !void {
    rect.draw(Style.background);
    switch (state.tabs.items[state.current_tab]) {
        .main => try uiTabMain(allocator, rect, state),
        .new_food => |*tab| try uiTabNewFood(allocator, io, rect, state, tab),
    }
}

fn uiEditPanel(allocator: std.mem.Allocator, rect: rl.Rectangle, state: *State) !void {
    rect.draw(.light_gray);
    UI.start(rect.toVector2().addValue(Style.padding), .{});
    ui.label("name", .{});
    // const id = state.table_current_row.?;
    if (try ui.input(
        allocator,
        &state.edit.name,
        rect.width - Style.padding,
        .{},
    )) {
        std.debug.print("updateName\n", .{});
        // state.database.updateName(@intCast(id + 1), state.edit.name.items);
    }
    ui.label("energy", .{});
    if (try ui.input(
        allocator,
        &state.edit.energy,
        rect.width - Style.padding,
        .{},
    )) {
        std.debug.print("updateEnergy\n", .{});
        // const energy: ?f64 = std.fmt.parseFloat(f64, state.edit.energy.items) catch null;
        // if (energy) |e| {
        //     state.database.updateEnergy(@intCast(id + 1), e);
        // }
    }
}

pub fn main(init: std.process.Init) !void {
    var state: State = try .init(init.gpa, init.io);
    defer state.deinit(init.gpa);

    ui.init();
    defer ui.deinit();

    rl.ConfigFlags.set(.{ .window_resizable = true });

    var width: i32 = 1280;
    var height: i32 = 720;

    rl.Window.init(width, height, "umai");
    defer rl.Window.close();

    rl.setTargetFPS(60);

    rl.Key.setExit(.caps_lock);

    while (!rl.Window.shouldClose()) {
        ui.frameStart();
        defer ui.frameEnd();

        // update
        try state.update(init.gpa);

        if (rl.Window.isResized()) {
            width = rl.getScreenWidth();
            height = rl.getScreenHeight();
        }

        // draw
        rl.beginDrawing();
        rl.clearBackground(Style.clear_color);
        const bar_rect = blk: {
            UI.start(.{ .x = Style.padding, .y = Style.padding }, .{});
            if (ui.button("add a food", .{})) try state.addTab(init.gpa);
            break :blk rl.Rectangle{ .height = ui.last_rect.height + Style.padding * 2 };
        };
        const max_width: f32 = cast(f32, width) / 6;
        try uiTabs(.{ .x = 0, .y = bar_rect.height, .width = max_width }, &state);
        try uiTab(init.gpa, init.io, .{
            .x = max_width,
            .y = bar_rect.height,
            .width = cast(f32, width - (cast(i32, max_width) + Style.padding)),
            .height = cast(f32, height) - (bar_rect.height + Style.padding),
        }, &state);
        rl.endDrawing();
    }
}
