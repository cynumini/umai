const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const raylib = b.createModule(.{
        .root_source_file = b.path("src/raylib.zig"),
        .target = target,
        .link_libc = true,
    });
    raylib.linkSystemLibrary("raylib", .{});

    const sqlite3 = b.createModule(.{
        .root_source_file = b.path("src/sqlite3.zig"),
        .target = target,
        .link_libc = true,
    });
    sqlite3.linkSystemLibrary("sqlite3", .{});

    const exe = b.addExecutable(.{
        .name = "umai",
        // .use_llvm = true,
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/main.zig"),
            .target = target,
            .optimize = optimize,
            .imports = &.{
                .{ .name = "raylib", .module = raylib },
                .{ .name = "sqlite3", .module = sqlite3 },
            },
        }),
    });
    b.installArtifact(exe);
    const run_step = b.step("run", "Run the app");
    const run_cmd = b.addRunArtifact(exe);
    run_step.dependOn(&run_cmd.step);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }
    const raylib_tests = b.addTest(.{
        .root_module = raylib,
    });
    const run_raylib_tests = b.addRunArtifact(raylib_tests);
    const exe_tests = b.addTest(.{
        .root_module = exe.root_module,
    });
    const run_exe_tests = b.addRunArtifact(exe_tests);
    const test_step = b.step("test", "Run tests");
    test_step.dependOn(&run_raylib_tests.step);
    test_step.dependOn(&run_exe_tests.step);
}
