const std = @import("std");
const sqlite3 = @import("sqlite3");
const ui = @import("ui.zig");

const Self = @This();

const Food = struct {
    id: i64,
    created: i64,
    name: [:0]const u8,
    energy: f64,

    fn init(allocator: std.mem.Allocator, id: i64, created: i64, name: [:0]const u8, energy: f64) !Food {
        return .{
            .id = id,
            .created = created,
            .name = try allocator.dupeSentinel(u8, name, 0),
            .energy = energy,
        };
    }

    fn deinit(self: Food, allocator: std.mem.Allocator) void {
        allocator.free(self.name);
    }

    fn row(self: Food, allocator: std.mem.Allocator) ![4][:0]const u8 {
        return .{
            try std.fmt.allocPrintSentinel(allocator, "{}", .{self.id}, 0),
            try std.fmt.allocPrintSentinel(allocator, "{}", .{self.created}, 0),
            try allocator.dupeSentinel(u8, self.name, 0),
            try std.fmt.allocPrintSentinel(allocator, "{}", .{self.energy}, 0),
        };
    }
};

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

    pub fn updateName(self: *Database, index: i64, name: []const u8) void {
        const stmt = self.db.prepare("UPDATE food SET name = ? WHERE id = ?");
        defer stmt.deinit();

        try stmt.bindText(1, name, .static);
        try stmt.bindInt64(2, index);

        std.debug.assert(stmt.step() == .done);
        std.debug.assert(stmt.step() == .done);
        self.need_update = true;
        // std.debug.print("{s}\n", .{name});
    }

    pub fn updateEnergy(self: *Database, index: i64, energy: f64) void {
        const stmt = self.db.prepare("UPDATE food SET energy = ? WHERE id = ?");
        defer stmt.deinit();

        try stmt.bindDouble(1, energy);
        try stmt.bindInt64(2, index);
        std.debug.assert(stmt.step() == .done);
        self.need_update = true;
    }
};

const Edit = struct {
    name: ui.Text,
    energy: ui.Text,

    pub fn init(allocator: std.mem.Allocator) !Edit {
        return .{
            .name = try .init(allocator, "name of food", null),
            .energy = try .init(allocator, "energy of food", null),
        };
    }

    pub fn deinit(self: *Edit, allocator: std.mem.Allocator) void {
        self.name.deinit(allocator);
        self.energy.deinit(allocator);
    }
};

const Tab = Edit;

edit: Edit,
database: Database,
table_current_row: ?usize,
foods_str: std.ArrayList([4][:0]const u8),
foods: std.ArrayList(Food),
tabs: std.ArrayList(Tab),

pub fn init(allocator: std.mem.Allocator, io: std.Io) !Self {
    return .{
        .edit = try .init(allocator),
        .tabs = .empty,
        .database = try .init(io),
        .table_current_row = null,
        .foods_str = .empty,
        .foods = .empty,
    };
}

pub fn deinit(self: *Self, allocator: std.mem.Allocator) void {
    self.edit.deinit(allocator);

    for (self.foods.items) |food| food.deinit(allocator);
    self.foods.deinit(allocator);

    for (self.foods_str.items) |food| for (0..food.len) |i| allocator.free(food[i]);
    self.foods_str.deinit(allocator);

    for (self.tabs.items) |*tab| {
        tab.deinit(allocator);
    }
    self.tabs.deinit(allocator);
}

pub fn update(self: *Self, allocator: std.mem.Allocator) !void {
    if (self.database.need_update) {
        // clear
        for (self.foods.items) |food| food.deinit(allocator);
        self.foods.clearRetainingCapacity();
        for (self.foods_str.items) |food| for (0..food.len) |i| allocator.free(food[i]);
        self.foods_str.clearRetainingCapacity();
        // update
        self.database.need_update = false;
        try self.foods_str.append(allocator, .{
            try allocator.dupeSentinel(u8, "id", 0),
            try allocator.dupeSentinel(u8, "created", 0),
            try allocator.dupeSentinel(u8, "name", 0),
            try allocator.dupeSentinel(u8, "energy", 0),
        });
        const stmt = self.database.db.prepare("SELECT * FROM food");
        defer stmt.deinit();
        var rc = stmt.step();
        while (rc != .done) : (rc = stmt.step()) {
            try self.foods.append(allocator, try .init(
                allocator,
                stmt.columnInt64(0),
                stmt.columnInt64(1),
                stmt.columnText(2),
                stmt.columnDouble(3),
            ));
            const food = self.foods.getLast();
            try self.foods_str.append(allocator, try food.row(allocator));
        }
    }
}
