lexer: Lexer,
current: ?Token = null,
next: ?Token = null,

pub fn init(lexer: Lexer) Parser {
    var self: Parser = .{ .lexer = lexer };

    _ = self.advance();
    _ = self.advance();
}

fn advance(self: *Parser) ?Token {
    const cur = self.current;
    self.current = self.next;
    self.next = self.lexer.next();

    return cur;
}

pub fn statement(self: *Parser) ?Statement {
    const tok = self.current orelse return null;
}

const Parser = @import("Parser.zig");

const std = @import("std");

const Token = @import("Token.zig");
const Lexer = @import("Lexer.zig");

const ast = @import("ast.zig");
const Statement = ast.Statement;
const Expression = ast.Expression;
