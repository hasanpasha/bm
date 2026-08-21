lexer: Lexer,
current: ?Token = null,
next: ?Token = null,

pub fn init(lexer: Lexer) Parser {
    var self: Parser = .{ .lexer = lexer };

    _ = self.advance();
    _ = self.advance();

    return self;
}

fn advance(self: *Parser) ?Token {
    const cur = self.current;
    self.current = self.next;
    self.next = self.lexer.next();

    return cur;
}

fn current_unwrap(self: *Parser) Token {
    return self.current orelse panic("{s}:{f}: unexpected end of token stream", .{
        self.lexer.source_name,
        self.lexer.current_loc,
    });
}

fn next_unwrap(self: *Parser) Token {
    return self.next orelse panic("{s}:{f}: unexpected end of token stream", .{
        self.lexer.source_name,
        self.lexer.current_loc,
    });
}

fn expect_token(self: *Parser, tok_type: Token.Type) Token {
    if (self.current_unwrap().type != tok_type) panic("{s}:{f}: unexpect token {f}, expected {t}", .{
        self.lexer.source_name,
        self.current_unwrap().location,
        self.current_unwrap(),
        tok_type,
    });

    return self.advance() orelse unreachable;
}

fn munch_token(self: *Parser, tok_type: Token.Type) void {
    _ = self.expect_token(tok_type);
}

fn integer_expr(self: *Parser) Expression {
    const integer_tok = self.expect_token(.integer);
    const integer = std.fmt.parseUnsigned(u64, integer_tok.lexeme, 10) catch unreachable;

    return .{ .integer = integer };
}

fn float_expr(self: *Parser) Expression {
    const float_tok = self.expect_token(.float);
    const float = std.fmt.parseFloat(f64, float_tok.lexeme) catch unreachable;

    return .{ .float = float };
}

fn variable_expr(self: *Parser) Expression {
    const ident = self.expect_token(.ident);

    return .{ .variable = ident.lexeme };
}

fn expression(self: *Parser) ?Expression {
    if (self.current == null or self.current_unwrap().type == .new_line) return null;

    return switch (self.current_unwrap().type) {
        .integer => self.integer_expr(),
        .float => self.float_expr(),
        .ident => self.variable_expr(),
        else => panic("{s}:{f}: unexpected expression token: {f}", .{
            self.lexer.source_name,
            self.current_unwrap().location,
            self.current_unwrap(),
        }),
    };
}

fn label_stmt(self: *Parser) Statement {
    const ident = self.expect_token(.ident);
    self.munch_token(.colon);

    return .{ .label = ident.lexeme };
}

fn inst_stmt(self: *Parser) Statement {
    const name = self.expect_token(.ident);
    const operand = self.expression();

    if (self.current != null) {
        self.munch_token(.new_line);
    } else {
        log.warn("{s}:{f}: instruction statement should end with a newline", .{
            self.lexer.source_name,
            self.lexer.current_loc,
        });
    }

    return .{ .instruction = .{ .name = name.lexeme, .operand = operand } };
}

pub fn statement(self: *Parser) ?Statement {
    const tok = self.current orelse return null;

    return switch (tok.type) {
        .ident => if (self.next != null and self.next_unwrap().type == .colon) self.label_stmt() else self.inst_stmt(),
        .new_line => blk: {
            while (self.current != null and self.current_unwrap().type == .new_line) {
                self.munch_token(.new_line);
            }
            break :blk self.statement();
        },
        else => panic("{s}:{f}: unexpected statement token: {f}", .{ self.lexer.source_name, tok.location, tok }),
    };
}

const Parser = @import("Parser.zig");

const std = @import("std");
const log = std.log.scoped(.parser);
const panic = std.debug.panic;
const alt = std.fmt.alt;

const Token = @import("Token.zig");
const Lexer = @import("Lexer.zig");

const ast = @import("ast.zig");
const Statement = ast.Statement;
const Expression = ast.Expression;
