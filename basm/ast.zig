pub const ExpressionKind = enum {
    integer,
    float,
    variable,
};

pub const Expression = union(ExpressionKind) {
    integer: u64,
    float: f64,
    variable: []const u8,
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
};

const Token = @import("Token.zig");
