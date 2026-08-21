const Options = struct {
    input: []const u8,

    const InnerOptions = struct {
        input: ?[]const u8 = null,
        help: bool = false,

        pub const shorthands = .{
            .i = "input",
            .h = "help",
        };

        pub const meta = .{
            .option_docs = .{
                .input = "input file path",
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

        return .{ .input = input };
    }
};

pub fn main(init: std.process.Init) !void {
    const options: Options = try .parse(init);

    var program: Program = try .init(init.gpa, 1024);
    defer program.deinit();

    try program.load_from_file(init.io, options.input);

    var buffer: [1024]u8 = undefined;
    const stdout: std.Io.File = .stdout();
    var stdout_writer = stdout.writer(init.io, &buffer);
    const writer = &stdout_writer.interface;

    for (program.insts.items) |inst| {
        try writer.print("{t}", .{inst.type});
        if (inst.type.has_operand()) {
            try writer.print(" {}", .{inst.operand});
        }
        try writer.print("\n", .{});
    }

    try writer.flush();
}

const std = @import("std");

const argsParser = @import("args");

const bm = @import("bm");
const Program = bm.Program;
