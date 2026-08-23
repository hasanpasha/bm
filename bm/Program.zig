insts: std.ArrayList(Inst),
alloc: Allocator,

pub fn init(allocator: Allocator) Allocator.Error!Program {
    return .{ .insts = try .initCapacity(allocator, 1024), .alloc = allocator };
}

pub fn deinit(self: *Program) void {
    self.insts.deinit(self.alloc);
}

pub fn size(self: *const Program) usize {
    return self.insts.items.len;
}

pub fn push(self: *Program, inst: Inst) !void {
    try self.insts.append(self.alloc, inst);
}

pub const LoadFromFileError = (Allocator.Error || Io.File.OpenError || Io.Reader.Error);

pub fn load_from_file(filepath: []const u8, io: std.Io, allocator: Allocator) LoadFromFileError!Program {
    const file = try Io.Dir.cwd().openFile(io, filepath, .{});
    defer file.close(io);

    var buffer: [64]u8 = undefined;
    var filereader = file.reader(io, &buffer);
    const reader = &filereader.interface;

    var program: Program = try .init(allocator);
    while (try Inst.read(reader)) |inst|
        try program.push(inst);

    try program.insts.shrinkToLen(allocator);

    return program;
}

pub const SaveToFileError = (Io.File.OpenError || Io.Writer.Error);

pub fn save_to_file(self: *const Program, filepath: []const u8, io: std.Io) SaveToFileError!void {
    const file = try std.Io.Dir.cwd().createFile(io, filepath, .{});
    defer file.close(io);

    var buffer: [64]u8 = undefined;
    var filewriter = file.writer(io, &buffer);
    const writer = &filewriter.interface;

    for (self.insts.items) |inst|
        try inst.write(writer);

    try writer.flush();
}

const Program = @This();

const std = @import("std");
const Io = std.Io;
const Allocator = std.mem.Allocator;

const Inst = @import("inst.zig").Inst;
