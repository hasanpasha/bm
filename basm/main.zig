pub fn main(init: std.process.Init) !void {
    var args = init.minimal.args.iterate();

    _ = args.next(); // skip the program name

    const input_file_path = args.next() orelse panic("No input file provided", .{});
    const output_file_path = args.next() orelse panic("No output file provided", .{});

    var basm: Basm = try .init(init.gpa);
    defer basm.deinit();

    try basm.assemble_file(input_file_path, init.io);

    for (basm.program.insts.items) |inst| {
        log.info("{f}", .{inst});
    }

    try basm.program.save_to_file(init.io, output_file_path);
}

const Basm = @import("Basm.zig");
const Lexer = @import("Lexer.zig");
const Parser = @import("Parser.zig");

const std = @import("std");
const log = std.log.scoped(.basm);
const panic = std.debug.panic;
