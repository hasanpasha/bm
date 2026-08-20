pub fn main(init: std.process.Init) !void {
    var args = init.minimal.args.iterate();

    _ = args.next(); // skip the program name

    const input_file_path = args.next() orelse panic("No input file provided", .{});
    const source = try std.Io.Dir.cwd().readFileAlloc(init.io, input_file_path, init.gpa, .unlimited);
    defer init.gpa.free(source);

    var lexer: Lexer = .{ .source = source };

    while (lexer.next()) |token| {
        log.info("{s}:{f}", .{ input_file_path, std.fmt.alt(token, .source_fmt) });
    }
}

const Lexer = @import("Lexer.zig");

const std = @import("std");
const log = std.log.scoped(.basm);
const panic = std.debug.panic;
