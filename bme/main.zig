pub fn main(init: std.process.Init) !void {
    var machine: Machine = try Machine.init(init.gpa);
    defer machine.deinit();

    try machine.program.load_from_file(init.io, "test.bm");
    try machine.execute_program(.{ .limited = 40 });
}

const std = @import("std");

const bm = @import("bm");
const Machine = bm.Machine;
const Inst = bm.inst.Inst;
