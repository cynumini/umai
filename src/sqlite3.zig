const std = @import("std");
pub const __helpers = @import("std").zig.c_translation.helpers;

const Result = enum(c_int) {
    ok = 0,
};

pub const sqlite3_stmt = opaque {
    extern fn sqlite3_step(?*sqlite3_stmt) c_int;
    extern fn sqlite3_column_count(pStmt: ?*sqlite3_stmt) c_int;
    extern fn sqlite3_column_type(?*sqlite3_stmt, iCol: c_int) c_int;
    extern fn sqlite3_column_name(?*sqlite3_stmt, N: c_int) [*:0]const u8;
    extern fn sqlite3_column_int(?*sqlite3_stmt, iCol: c_int) i32;
    extern fn sqlite3_column_int64(?*sqlite3_stmt, iCol: c_int) i64;
    extern fn sqlite3_column_text(?*sqlite3_stmt, iCol: c_int) [*:0]const u8;
    extern fn sqlite3_column_double(?*sqlite3_stmt, iCol: c_int) f64;
    extern fn sqlite3_finalize(pStmt: ?*sqlite3_stmt) c_int;
};

pub const sqlite3 = opaque {
    extern fn sqlite3_close(?*sqlite3) c_int;
    extern fn sqlite3_exec(?*sqlite3, sql: [*:0]const u8, callback: ?*const fn (?*anyopaque, c_int, [*c][*c]u8, [*c][*c]u8) callconv(.c) c_int, ?*anyopaque, errmsg: *[*:0]u8) c_int;
    extern fn sqlite3_prepare_v2(db: ?*sqlite3, zSql: [*c]const u8, nByte: c_int, ppStmt: [*c]?*sqlite3_stmt, pzTail: [*c][*c]const u8) c_int;
};

extern fn sqlite3_free(?*anyopaque) void;
extern fn sqlite3_open(filename: [*:0]const u8, ppDb: *?*sqlite3) c_int;

pub const Statement = struct {
    const StatementResult = enum(c_int) {
        row = 100,
        done = 101,
    };

    stmt: *sqlite3_stmt,

    pub fn deinit(self: Statement) void {
        std.debug.assert(self.stmt.sqlite3_finalize() == @intFromEnum(Result.ok));
    }

    pub fn step(self: Statement) StatementResult {
        return @enumFromInt(self.stmt.sqlite3_step());
    }

    pub fn columnCount(self: Statement) usize {
        return @intCast(self.stmt.sqlite3_column_count());
    }

    const ColumnType = enum(c_int) {
        integer = 1,
        float = 2,
        text = 3,
    };

    pub fn columnType(self: Statement, column: usize) ColumnType {
        return @enumFromInt(self.stmt.sqlite3_column_type(@intCast(column)));
    }

    pub fn columnName(self: Statement, column: usize) [:0]const u8 {
        const name = self.stmt.sqlite3_column_name(@intCast(column));
        return name[0..std.mem.len(name) :0];
    }

    pub fn columnInt(self: Statement, column: usize) i32 {
        return self.stmt.sqlite3_column_int(@intCast(column));
    }

    pub fn columnInt64(self: Statement, column: usize) i64 {
        return self.stmt.sqlite3_column_int64(@intCast(column));
    }

    pub fn columnText(self: Statement, column: usize) [:0]const u8 {
        const text = self.stmt.sqlite3_column_text(@intCast(column));
        return text[0..std.mem.len(text) :0];
    }

    pub fn columnDouble(self: Statement, column: usize) f64 {
        return self.stmt.sqlite3_column_double(@intCast(column));
    }

    pub const sqlite3_destructor_type = ?*const fn (?*anyopaque) callconv(.c) void;

    const Lifetime = enum(c_int) {
        static = 0,
        transient = 1,
    };

    extern fn sqlite3_bind_text(?*sqlite3_stmt, c_int, [*]const u8, c_int, ?*const fn (?*anyopaque) callconv(.c) void) c_int;
    pub fn bindText(self: Statement, index: i32, value: []const u8, lifetime: Lifetime) !void {
        const result = sqlite3_bind_text(
            self.stmt,
            index,
            value.ptr,
            @intCast(value.len),
            __helpers.cast(sqlite3_destructor_type, @intFromEnum(lifetime)),
        );
        std.debug.assert(result == @intFromEnum(Result.ok));
    }

    extern fn sqlite3_bind_int64(?*sqlite3_stmt, c_int, i64) c_int;
    pub fn bindInt64(self: Statement, index: i32, value: i64) !void {
        const result = sqlite3_bind_int64(self.stmt, index, value);
        std.debug.assert(result == @intFromEnum(Result.ok));
    }

    extern fn sqlite3_bind_double(?*sqlite3_stmt, c_int, f64) c_int;
    pub fn bindDouble(self: Statement, index: i32, value: f64) !void {
        const result = sqlite3_bind_double(self.stmt, index, value);
        std.debug.assert(result == @intFromEnum(Result.ok));
    }
};

pub const Database = struct {
    db: *sqlite3,

    pub fn init(path: [:0]const u8) Database {
        var db: ?*sqlite3 = null;
        std.debug.assert(sqlite3_open(path, &db) == @intFromEnum(Result.ok));
        return .{ .db = db.? };
    }

    pub fn deinit(self: Database) void {
        std.debug.assert(self.db.sqlite3_close() == @intFromEnum(Result.ok));
    }

    pub fn exec(self: Database, sql: [:0]const u8) void {
        var errmsg: [*:0]u8 = undefined;
        if (self.db.sqlite3_exec(sql, null, null, &errmsg) != @intFromEnum(Result.ok)) {
            std.debug.print("SQL: {s}\n", .{errmsg});
            sqlite3_free(errmsg);
            unreachable;
        }
    }

    pub fn prepare(self: Database, sql: [:0]const u8) Statement {
        var stmt: ?*sqlite3_stmt = null;
        const rc = self.db.sqlite3_prepare_v2(sql, @intCast(sql.len), &stmt, null);
        std.debug.assert(rc == @intFromEnum(Result.ok));
        return .{ .stmt = stmt.? };
    }
};
