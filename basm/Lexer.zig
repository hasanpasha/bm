pub const TokenType = enum {
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

pub const Token = struct {
    type: TokenType,
    lexeme: []const u8,
    location: Location,

    pub fn source_fmt(self: Token, writer: *std.Io.Writer) std.Io.Writer.Error!void {
        try writer.print("{f}:{f}", .{ self.location, self });
    }

    pub fn format(self: Token, writer: *std.Io.Writer) std.Io.Writer.Error!void {
        try writer.print("{t}('{s}')", .{ self.type, self.lexeme });
    }
};

source: []const u8,
start_pos: usize = 0,
current_pos: usize = 0,
start_loc: Location = .start,
current_loc: Location = .start,

fn is_at_end(self: *Lexer) bool {
    return self.current_pos >= self.source.len;
}

fn current_char(self: *Lexer) u8 {
    return self.source[self.current_pos];
}

fn current_lexeme(self: *Lexer) []const u8 {
    return self.source[self.start_pos..self.current_pos];
}

fn advance(self: *Lexer) void {
    if (self.is_at_end()) return;

    const c = self.current_char();
    self.current_pos += 1;

    if (c == '\n') {
        self.current_loc.line += 1;
        self.current_loc.column = 1;
    } else {
        self.current_loc.column += 1;
    }
}

fn skip_whitespace(self: *Lexer) void {
    while (!self.is_at_end() and std.ascii.isWhitespace(self.current_char()) and self.current_char() != '\n') {
        self.advance();
    }

    self.start_pos = self.current_pos;
    self.start_loc = self.current_loc;
}

fn skip_comments(self: *Lexer) void {
    while (!self.is_at_end() and self.current_char() == '#') {
        while (!self.is_at_end() and self.current_char() != '\n') {
            self.advance();
        }

        // skip the newline character after the comment
        if (!self.is_at_end()) {
            self.advance();
        }
    }
}

fn is_ident_start(c: u8) bool {
    return std.ascii.isAlphabetic(c) or c == '_';
}

fn is_ident_part(c: u8) bool {
    return std.ascii.isAlphanumeric(c) or c == '_';
}

fn is_numeric(c: u8) bool {
    return std.ascii.isDigit(c) or c == '.';
}

fn ident(self: *Lexer) Token {
    while (!self.is_at_end() and is_ident_part(self.current_char())) {
        self.advance();
    }

    return self.token(.ident);
}

fn number(self: *Lexer) Token {
    var encountered_dot: bool = false;
    while (!self.is_at_end() and is_numeric(self.current_char())) {
        if (self.current_char() == '.') {
            if (encountered_dot) break;
            encountered_dot = true;
        }
        self.advance();
    }

    return self.token(if (encountered_dot) .float else .integer);
}

fn token(self: *Lexer, token_type: TokenType) Token {
    return Token{ .type = token_type, .lexeme = self.current_lexeme(), .location = self.start_loc };
}

fn advance_and_return(self: *Lexer, token_type: TokenType) Token {
    self.advance();
    return self.token(token_type);
}

pub fn next(self: *Lexer) ?Token {
    self.skip_whitespace();

    if (self.is_at_end()) return null;

    const c = self.current_char();

    return switch (c) {
        ':' => self.advance_and_return(.colon),
        '\n' => self.advance_and_return(.new_line),
        '#' => blk: {
            self.skip_comments();
            break :blk self.next();
        },
        else => if (is_ident_start(c))
            self.ident()
        else if (is_numeric(c))
            self.number()
        else
            panic("unexpected character: {c}", .{c}),
    };
}

const Lexer = @This();

const std = @import("std");
const panic = std.debug.panic;

test "numbers" {
    const source = "123 456.789";
    var lexer = Lexer{ .source = source };

    const token1 = lexer.next() orelse unreachable;
    try std.testing.expectEqual(token1.type, .integer);
    try std.testing.expectEqualStrings(token1.lexeme, "123");

    const token2 = lexer.next() orelse unreachable;
    try std.testing.expectEqual(token2.type, .float);
    try std.testing.expectEqualStrings(token2.lexeme, "456.789");
}
