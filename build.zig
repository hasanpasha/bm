const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.resolveTargetQuery(.{});
    const optimize = b.standardOptimizeOption(.{});

    const basm_mod = b.addModule("basm", .{
        .root_source_file = b.path("basm/root.zig"),
        .target = target,
        .optimize = optimize,
    });

    const bm_mod = b.addModule("bm", .{
        .root_source_file = b.path("BM.zig"),
        .target = target,
        .optimize = optimize,
    });

    const bm_exe = b.addExecutable(.{
        .name = "bm",
        .root_module = bm_mod,
    });

    const basm_exe = b.addExecutable(.{
        .name = "basm",
        .root_module = basm_mod,
    });

    b.installArtifact(bm_exe);
    b.installArtifact(basm_exe);

    const bm_run = b.addRunArtifact(bm_exe);
    if (b.args) |args| bm_run.addArgs(args);

    const basm_run = b.addRunArtifact(basm_exe);
    if (b.args) |args| basm_run.addArgs(args);

    b.step("run-bm", "run bm").dependOn(&bm_run.step);
    b.step("run-basm", "run basm").dependOn(&basm_run.step);
}
