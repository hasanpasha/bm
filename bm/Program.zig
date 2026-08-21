insts: std.ArrayList(Inst),
allocator: std.mem.Allocator,

pub fn init(allocator: std.mem.Allocator, capacity: usize) !Program {
    return .{ .insts = try .initCapacity(allocator, capacity), .allocator = allocator };
}

pub fn deinit(self: *Program) void {
    self.insts.deinit(self.allocator);
}

pub fn push(self: *Program, inst: Inst) !void {
    try self.insts.append(self.allocator, inst);
}

pub fn push_many(self: *Program, insts: []const Inst) !void {
    try self.insts.appendSlice(self.allocator, insts);
}

pub fn get_or_null(self: *Program, idx: usize) ?Inst {
    if (idx >= self.insts.items.len)
        return null;

    return self.insts.items[idx];
}

var buffer: [1024]u8 = undefined;

pub fn save_to_file(self: *const Program, io: std.Io, filepath: []const u8) !void {
    const file = try std.Io.Dir.cwd().createFile(io, filepath, .{ .truncate = true });
    defer file.close(io);

    var filewriter = file.writer(io, &buffer);
    const writer = &filewriter.interface;

    for (self.insts.items) |inst| {
        try writer.writeStruct(inst, .little);
    }

    try writer.flush();
}

pub fn load_from_file(self: *Program, io: std.Io, filepath: []const u8) !void {
    const file = try std.Io.Dir.cwd().openFile(io, filepath, .{});
    defer file.close(io);

    var filereader = file.reader(io, &buffer);
    const reader = &filereader.interface;

    while (true) {
        const inst = reader.takeStruct(Inst, .little) catch |err| switch (err) {
            error.EndOfStream => break,
            else => return err,
        };

        try self.push(inst);
    }
}

const Program = @This();

const std = @import("std");

const Inst = @import("inst.zig").Inst;
