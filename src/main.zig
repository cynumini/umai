const std = @import("std");
const rl = @import("raylib");
const sqlite3 = @import("sqlite3");

const UI = @import("ui.zig");

// fn callback(_: *UI, _: *UI.Node, data: ?*anyopaque) void {
//     if (data) |d| {
//         const visible: *bool = @ptrCast(@alignCast(d));
//         visible.* = !visible.*;
//     }
// }

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

    var table: std.ArrayList(Food) = .empty;
    defer {
        for (table.items) |food| {
            food.deinit(init.gpa);
        }
        table.deinit(init.gpa);
    }

    //var scroll: i32 = 0;
    var tab_index: usize = 0;

    rl.ConfigFlags.set(.{ .window_resizable = true });
    const window = rl.Window.init(width, height, "umai");
    defer window.deinit();

    rl.setTargetFPS(60);

    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    var v = rl.RenderTexture.init(0, 0);
    defer v.deinit();

    while (!window.shouldClose()) {
        var ui = UI{};
        defer _ = arena.reset(.retain_capacity);

        // update
        if (database_need_update) {
            // clear table
            for (table.items) |food| food.deinit(init.gpa);
            table.clearRetainingCapacity();
            // end clear table

            database_need_update = false;
            const stmt = database.prepare("SELECT * FROM food");
            defer stmt.deinit();
            var rc = stmt.step();
            while (rc != .done) : (rc = stmt.step()) {
                try table.append(init.gpa, try Food.init(
                    init.gpa,
                    @intCast(stmt.columnInt64(0)),
                    stmt.columnInt64(1),
                    stmt.columnText(2),
                    stmt.columnDouble(3),
                ));
            }
        }

        if (window.isResized()) {
            width = rl.getScreenWidth();
            height = rl.getScreenHeight();
        }

        try ui.begin(allocator, .{
            .width = .{ .fixed = width },
            .height = .{ .fixed = height },
            .direction = .top_to_bottom,
        });
        try ui.button(allocator, "add a food", .{});

        var tc = try ui.tabContainerBegin(allocator, &tab_index);
        {
            try ui.tabBegin(allocator, &tc, "main", .{
                .background = .red,
                .direction = .top_to_bottom,
            });
            {
                try ui.tableBegin(allocator, .{});
                for (table.items) |food| {
                    try ui.rowBegin(allocator, .{});
                    try ui.label(allocator, food.name, .{});
                    ui.rowEnd();
                }
                ui.tableEnd();
            }

            try ui.tabEnd(allocator);
            inline for (&.{ rl.Color.green, rl.Color.blue, rl.Color{ .r = 0, .g = 255, .b = 255, .a = 255 } }) |color| {
                try ui.tabBegin(allocator, &tc, "new food", .{
                    .background = color,
                });
                try ui.tabEnd(allocator);
            }
        }
        try ui.tabContainerEnd(allocator, tc);

        try ui.end(allocator);

        // draw
        rl.beginDrawing();
        rl.clearBackground(.light_gray);
        try ui.draw();
        rl.endDrawing();
    }
}
