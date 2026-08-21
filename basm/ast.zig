pub const ExpressionKind = enum {
    integer,
    float,
    variable,
};

pub const Expression = union(ExpressionKind) {
    integer: u64,
    float: f64,
    variable: []const u8,

    pub fn format(self: Expression, writer: *std.Io.Writer) std.Io.Writer.Error!void {
        try writer.print("{t}(", .{std.meta.activeTag(self)});
        switch (self) {
            .integer => |int| try writer.print("{}", .{int}),
            .float => |float| try writer.print("{}", .{float}),
            .variable => |name| try writer.print("{s}", .{name}),
        }
        try writer.print(")", .{});
    }
};

pub const StatementKind = enum {
    label,
    instruction,
};

pub const Statement = union(StatementKind) {
    label: []const u8,
    instruction: Instruction,

    pub const Instruction = struct {
        name: []const u8,
        operand: ?Expression,
    };

    pub fn format(self: Statement, writer: *std.Io.Writer) std.Io.Writer.Error!void {
        try writer.print("{t}(", .{std.meta.activeTag(self)});
        switch (self) {
            .label => |name| try writer.print("{s}", .{name}),
            .instruction => |inst| try writer.print("name={s}, operand={?f}", .{ inst.name, inst.operand }),
        }
        try writer.print(")", .{});
    }
};

const std = @import("std");

const Token = @import("Token.zig");
