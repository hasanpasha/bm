type: Type,
lexeme: []const u8,
location: Location,

pub const Type = enum {
    ident,
    integer,
    float,
    new_line,
    colon,
};

pub const Location = struct {
    line: usize,
    column: usize,

    pub const start: Location = .{ .line = 1, .column = 1 };

    pub fn format(self: Location, writer: *std.Io.Writer) std.Io.Writer.Error!void {
        try writer.print("{}:{}", .{ self.line, self.column });
    }
};

pub fn source_fmt(self: Token, writer: *std.Io.Writer) std.Io.Writer.Error!void {
    try writer.print("{f}:{f}", .{ self.location, self });
}

pub fn format(self: Token, writer: *std.Io.Writer) std.Io.Writer.Error!void {
    try writer.print("{t}('{s}')", .{ self.type, self.lexeme });
}

const Token = @This();

const std = @import("std");
