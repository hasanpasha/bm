variables: std.StringHashMap(Expression),

allocator: std.mem.Allocator,

pub const Error = error{
    undefined_variables,
    already_defined,
};

pub fn init(allocator: std.mem.Allocator) !Basm {
    const variables: std.StringHashMap(Expression) = .init(allocator);

    return .{
        .variables = variables,
        .allocator = allocator,
    };
}

pub fn deinit(self: *Basm) void {
    self.variables.deinit();
}

fn resolve_expression(self: *Basm, expr: Expression) Error!Inst.Word {
    return switch (expr) {
        .integer => |int| int,
        .float => panic("unimplemented", .{}),
        .variable => |name| blk: {
            const value = self.variables.get(name) orelse return Error.undefined_variables;
            break :blk try self.resolve_expression(value);
        },
    };
}

pub fn assemble_file(self: *Basm, file_path: []const u8, io: std.Io, program: *Program) !void {
    const source = try std.Io.Dir.cwd().readFileAlloc(io, file_path, self.allocator, .unlimited);
    defer self.allocator.free(source);

    var parser: Parser = .init(.{ .source_name = file_path, .source = source });

    while (parser.statement()) |stmt| {
        switch (stmt) {
            .label => |name| {
                if (self.variables.contains(name)) return Error.already_defined;
                try self.variables.put(name, .{ .integer = program.size() });
            },
            .instruction => |basm_inst| {
                if (std.meta.stringToEnum(Inst.Type, basm_inst.name)) |inst_type| {
                    var inst: Inst = .{ .type = inst_type };

                    if (inst_type.has_operand()) {
                        if (basm_inst.operand) |operand| {
                            inst.operand = try self.resolve_expression(operand);
                        } else {
                            panic("{t} requires an operand", .{inst_type});
                        }
                    }

                    try program.push(inst);
                } else {
                    panic("unknown instruction '{s}'", .{basm_inst.name});
                }
            },
        }
    }
}

const Basm = @This();

const std = @import("std");
const panic = std.debug.panic;

const bm = @import("bm");
const Inst = bm.Inst;
const Program = bm.Program;

const ast = @import("ast.zig");
const Expression = ast.Expression;
const Lexer = @import("Lexer.zig");
const Parser = @import("Parser.zig");
