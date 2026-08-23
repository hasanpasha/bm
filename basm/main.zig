const Options = struct {
    input: []const u8,
    output: []const u8,

    const InnerOptions = struct {
        input: ?[]const u8 = null,
        output: ?[]const u8 = null,
        help: bool = false,

        pub const shorthands = .{
            .i = "input",
            .o = "output",
            .h = "help",
        };

        pub const meta = .{
            .option_docs = .{
                .input = "input file path",
                .output = "output file path",
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

        const output = options.options.output orelse
            print_usage(init, "output");

        return .{
            .input = input,
            .output = output,
        };
    }
};

pub fn main(init: std.process.Init) !void {
    const options: Options = try .parse(init);

    var basm: Basm = try .init(init.gpa);
    defer basm.deinit();

    var program: Program = try .init(init.gpa);
    defer program.deinit();

    try basm.assemble_file(options.input, init.io, &program);

    try program.save_to_file(options.output, init.io);
}

const Basm = @import("Basm.zig");
const Lexer = @import("Lexer.zig");
const Parser = @import("Parser.zig");

const Program = @import("bm").Program;

const std = @import("std");
const log = std.log.scoped(.basm);
const panic = std.debug.panic;

const argsParser = @import("args");
