const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.resolveTargetQuery(.{});
    const optimize = b.standardOptimizeOption(.{});

    const args_mod = b.dependency("args", .{ .target = target, .optimize = optimize }).module("args");

    const bm_mod = b.addModule("bm", .{
        .root_source_file = b.path("bm/root.zig"),
        .target = target,
        .optimize = optimize,
    });

    const bme_exe = b.addExecutable(.{
        .name = "bme",
        .root_module = b.createModule(.{
            .root_source_file = b.path("bme/main.zig"),
            .target = target,
            .optimize = optimize,
            .imports = &.{
                .{ .name = "bm", .module = bm_mod },
                .{ .name = "args", .module = args_mod },
            },
        }),
    });

    const basm_exe = b.addExecutable(.{
        .name = "basm",
        .root_module = b.createModule(.{
            .root_source_file = b.path("basm/main.zig"),
            .target = target,
            .optimize = optimize,
            .imports = &.{
                .{ .name = "bm", .module = bm_mod },
                .{ .name = "args", .module = args_mod },
            },
        }),
    });

    b.installArtifact(bme_exe);
    b.installArtifact(basm_exe);

    const bme_run = b.addRunArtifact(bme_exe);
    const basm_run = b.addRunArtifact(basm_exe);

    if (b.args) |args| {
        bme_run.addArgs(args);
        basm_run.addArgs(args);
    }

    b.step("run-bme", "run bme").dependOn(&bme_run.step);
    b.step("run-basm", "run basm").dependOn(&basm_run.step);
}
