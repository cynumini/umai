const std = @import("std");
const cast = std.math.lossyCast;

const rl = @import("raylib");
const sqlite3 = @import("sqlite3");

const State = @import("state.zig");
const table = @import("table.zig");
const ui = @import("ui.zig");
const UI = ui.UI;
const Style = ui.Style;

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

    // init
    var current_tab: usize = 0;

    rl.Key.setExit(.q);

    while (!rl.Window.shouldClose()) {
        ui.frameStart();
        defer ui.frameEnd();
        // update
        if (rl.Window.isResized()) {
            width = rl.getScreenWidth();
            height = rl.getScreenHeight();
        }
        try state.update(init.gpa);

        // draw
        rl.beginDrawing();
        rl.clearBackground(.light_gray);
        UI.start(.{ .x = Style.padding, .y = Style.padding }, .{});
        if (ui.button("add a food", .{})) {
            try state.tabs.append(init.gpa, try .init(init.gpa));
        }
        const max_width = 128;
        // const margin = 1;
        // const font_size = 20;
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
        for (0..state.tabs.items.len + 1) |i| {
            const background: rl.Color = if (current_tab == i) .white else .gray;
            if (i == 0) {
                if (ui.button("main", .{
                    .border_color = background,
                    .background = background,
                    .width = max_width,
                })) current_tab = i;
            } else {
                const name = try state.tabs.items[i - 1].name.get(ui.allocator);
                const text: [:0]const u8 = blk: {
                    var len = name.len;
                    if (len > 0) {
                        var text: []u8 = try ui.allocator.dupeSentinel(u8, name, 0);
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
            const old_index = state.table_current_row;
            try table.table(.{
                .x = tab_background_rect.x,
                .y = tab_background_rect.y,
                .width = tab_background_rect.width / 2,
                .height = tab_background_rect.height,
            }, &state);

            if (old_index != state.table_current_row) {
                if (state.table_current_row) |i| {
                    try state.edit.name.set(init.gpa, state.foods_str.items[i + 1][2]);
                    try state.edit.energy.set(init.gpa, state.foods_str.items[i + 1][3]);
                }
            }

            if (state.table_current_row != null) {
                try uiEditPanel(init.gpa, .{
                    .x = tab_background_rect.x + tab_background_rect.width / 2,
                    .y = tab_background_rect.y,
                    .width = tab_background_rect.width / 2,
                    .height = tab_background_rect.height,
                }, &state);
            }
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
                &state.tabs.items[current_tab - 1].name,
                inputs_width,
                .{},
            );
            _ = try ui.input(
                init.gpa,
                &state.tabs.items[current_tab - 1].energy,
                inputs_width,
                .{},
            );
            UI.start(.{
                .x = tab_background_rect.x + tab_background_rect.width - Style.padding,
                .y = tab_background_rect.y + tab_background_rect.height - Style.padding,
            }, .{ .origin = .right_bottom, .direction = .left });
            if (ui.button("add", .{})) {
                const i = current_tab - 1;
                const name = try state.tabs.items[i].name.get(ui.allocator);
                const raw_energy = try state.tabs.items[i].energy.get(ui.allocator);
                const energy = try std.fmt.parseFloat(f32, raw_energy);
                const stmt = state.database.db.prepare("INSERT INTO food (created, name, energy) VALUES (?, ?, ?)");
                defer stmt.deinit();
                try stmt.bindInt64(1, std.Io.Clock.real.now(init.io).toSeconds());
                try stmt.bindText(2, name, .static);
                try stmt.bindDouble(3, energy);
                std.debug.assert(stmt.step() == .done);
                var tab = state.tabs.orderedRemove(i);
                tab.deinit(init.gpa);
                current_tab -= 1;
                state.database.need_update = true;
            }
            if (ui.button("cancel", .{})) {
                var tab = state.tabs.orderedRemove(current_tab - 1);
                tab.deinit(init.gpa);
                current_tab -= 1;
                state.database.need_update = true;
            }
        }
        rl.endDrawing();
    }
}
