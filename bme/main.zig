const Options = struct {
    input: []const u8,
    limit: Machine.Limit,

    const InnerOptions = struct {
        input: ?[]const u8 = null,
        limit: ?usize = null,
        help: bool = false,

        pub const shorthands = .{
            .i = "input",
            .l = "limit",
            .h = "help",
        };

        pub const meta = .{
            .option_docs = .{
                .input = "input file path",
                .limit = "number of iteration",
                .help = "show this help message",
            },
        };
    };

    fn print_usage(init: std.process.Init, err: ?[]const u8) noreturn {
        var buffer: [1024]u8 = undefined;
        const file: std.Io.File = if (err == null) .stdout() else .stderr();
        var filewriter = file.writer(init.io, &buffer);
        const writer = &filewriter.interface;

        argsParser.printHelp(InnerOptions, err orelse "", writer) catch {};
        writer.flush() catch {};

        if (err == null) {
            std.process.exit(0);
        } else {
            std.process.exit(1);
        }
    }

    pub fn parse(init: std.process.Init) !Options {
        const options = try argsParser.parseForCurrentProcess(InnerOptions, init, .print);
        defer options.deinit();

        if (options.options.help)
            print_usage(init, null);

        const input = options.options.input orelse
            print_usage(init, "input");

        const limit: Machine.Limit = if (options.options.limit) |limit| .{ .limited = limit } else .no_limit;

        return .{
            .input = input,
            .limit = limit,
        };
    }
};

pub fn main(init: std.process.Init) !void {
    const options: Options = try .parse(init);

    var machine: Machine = try Machine.init(init.gpa);
    defer machine.deinit();

    try machine.program.load_from_file(init.io, options.input);
    try machine.execute_program(options.limit);
}

const std = @import("std");
const panic = std.debug.panic;

const bm = @import("bm");
const Machine = bm.Machine;
const Inst = bm.Inst;

const argsParser = @import("args");
