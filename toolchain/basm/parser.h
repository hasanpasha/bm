#ifndef BASM_PARSER_H
#define BASM_PARSER_H

#include <stdarg.h>

#include "ast.h"
#include "lexer.h"
#include "token.h"

typedef struct BASM_PARSER {
  BasmToken current, next;
  BasmLexer lexer;
} BasmParser;

static BasmToken basm_parser_advance(BasmParser *parser) {
  BasmToken tok = parser->current;
  parser->current = parser->next;
  parser->next = basm_lexer_next_token(&parser->lexer);
  return tok;
}

static BasmParser basm_parser_init(BasmLexer lexer) {
  BasmParser parser = {.lexer = lexer};
  (void)basm_parser_advance(&parser);
  (void)basm_parser_advance(&parser);
  return parser;
}

static BasmToken basm_parser_expect(BasmParser *parser, BasmTokenKind kind) {
  if (parser->current.kind != kind) {
    PANIC("'%s' doesn't match expected kind %s.",
          basm_token_kind_string(parser->current.kind),
          basm_token_kind_string(kind));
  }
  return basm_parser_advance(parser);
}

static void basm_parser_munch(BasmParser *parser, BasmTokenKind kind) {
  (void)basm_parser_expect(parser, kind);
}

typedef enum BASM_PARSE_PRECEDENCE {
  BASM_PARSE_PRECEDENCE_NONE,
  BASM_PARSE_PRECEDENCE_UNARY,
  BASM_PARSE_PRECEDENCE_PRIMARY,
} BasmParsePrecedence;

typedef BasmExpression (*BasmPrefixParseFn)(BasmParser *parser);
typedef BasmExpression (*BasmInfixParseFn)(BasmParser *parser,
                                           BasmExpression *lhs);

typedef struct BASM_PARSE_EXPR_RULE {
  BasmPrefixParseFn prefix;
  BasmInfixParseFn infix;
  BasmParsePrecedence precedence;
} BasmPareseExprRule;

static const BasmPareseExprRule *get_rule(BasmToken tok);
static BasmExpression parse_precedence(BasmParser *parser,
                                       BasmParsePrecedence prec);

static BasmExpression variable(BasmParser *parser) {
  BasmToken token = basm_parser_expect(parser, BASM_TOKEN_KIND_IDENTIFIER);
  return (BasmExpression){.kind = BASM_EXPRESSION_KIND_VARIABLE,
                          .u = {.variable = token}};
}

static BasmExpression integer(BasmParser *parser) {
  BasmToken token = basm_parser_expect(parser, BASM_TOKEN_KIND_INTEGER);
  const uint64_t num = sv_parse_ulong(token.lexeme);
  return (BasmExpression){.kind = BASM_EXPRESSION_KIND_INTEGER,
                          .u = {.integer = num}};
}

static BasmExpression _float(BasmParser *parser) {
  BasmToken token = basm_parser_expect(parser, BASM_TOKEN_KIND_FLOAT);
  const double num = sv_parse_double(token.lexeme);
  return (BasmExpression){.kind = BASM_EXPRESSION_KIND_FLOAT,
                          .u = {._float = num}};
}

static BasmExpression *alloc_expr(BasmExpression expr) {
  BasmExpression *p = (BasmExpression *)malloc(sizeof(BasmExpression));
  if (p == NULL)
    PANIC("failed to allocate an expression");

  *p = expr;
  return p;
}

static BasmExpression unary(BasmParser *parser) {
  BasmToken operator_token = basm_parser_advance(parser);
  BasmExpressionUnaryOperator operator;
  switch (operator_token.kind) {
  case BASM_TOKEN_KIND_MINUS:
    operator= BASM_EXPRESSION_UNARY_OPERATOR_MINUS;
    break;
  case BASM_TOKEN_KIND_PERCENT:
    PANIC("unimplemented");
  case BASM_TOKEN_KIND_STRING:
  case BASM_TOKEN_KIND_IDENTIFIER:
  case BASM_TOKEN_KIND_INTEGER:
  case BASM_TOKEN_KIND_FLOAT:
  case BASM_TOKEN_KIND_COLON:
  case BASM_TOKEN_KIND_NEW_LINE:
  case BASM_TOKEN_KIND_END_OF_INPUT:
    PANIC("unexpected unary operator");
  default:
    BM_UNREACHABLE();
  }

  BasmExpression operand =
      parse_precedence(parser, BASM_PARSE_PRECEDENCE_UNARY);

  return (BasmExpression){
      .kind = BASM_EXPRESSION_KIND_UNARY,
      .u = {.unary = {.operator= operator, .operand = alloc_expr(operand)}}};
}

static BasmExpression none_expr(BasmParser *parser) {
  (void)parser;
  return (BasmExpression){.kind = BASM_EXPRESSION_KIND_NONE};
}

// clang-format off
static const BasmPareseExprRule rules[] = {
    [BASM_TOKEN_KIND_IDENTIFIER] = {variable, NULL, BASM_PARSE_PRECEDENCE_NONE},
    [BASM_TOKEN_KIND_INTEGER] = {integer, NULL, BASM_PARSE_PRECEDENCE_NONE},
    [BASM_TOKEN_KIND_FLOAT] = {_float, NULL, BASM_PARSE_PRECEDENCE_NONE},
    [BASM_TOKEN_KIND_COLON] = {NULL, NULL, BASM_PARSE_PRECEDENCE_NONE},
    [BASM_TOKEN_KIND_MINUS] = {unary, NULL, BASM_PARSE_PRECEDENCE_UNARY},
    [BASM_TOKEN_KIND_NEW_LINE] = {none_expr, NULL, BASM_PARSE_PRECEDENCE_NONE},
    [BASM_TOKEN_KIND_END_OF_INPUT] = {none_expr, NULL, BASM_PARSE_PRECEDENCE_NONE},
};
// clang-format on

static const BasmPareseExprRule *get_rule(BasmToken tok) {
  return &rules[tok.kind];
}

static BasmExpression parse_precedence(BasmParser *parser,
                                       BasmParsePrecedence precedence) {
  const BasmPrefixParseFn prefix = get_rule(parser->current)->prefix;
  if (prefix == NULL)
    PANIC("expect expression for '" SV_FMT "'", SV_ARG(parser->current.lexeme));

  BasmExpression lhs = prefix(parser);

  while (precedence <= get_rule(parser->current)->precedence) {
    BasmInfixParseFn infix = get_rule(parser->current)->infix;
    if (infix == NULL)
      break;

    lhs = infix(parser, alloc_expr(lhs));
  }

  return lhs;
}

static BasmExpression basm_parser_expression(BasmParser *parser) {
  return parse_precedence(parser, BASM_PARSE_PRECEDENCE_UNARY);
}

static bool basm_parser_statement(BasmParser *parser,
                                  BasmStatement *statement_out) {
  switch (parser->current.kind) {
  case BASM_TOKEN_KIND_IDENTIFIER: {
    BasmToken label = {.kind = BASM_TOKEN_KIND_IDENTIFIER, .lexeme = {0}};
    if (parser->next.kind == BASM_TOKEN_KIND_COLON) {
      label = basm_parser_advance(parser);
      basm_parser_munch(parser, BASM_TOKEN_KIND_COLON);
    }

    while (parser->current.kind == BASM_TOKEN_KIND_NEW_LINE)
      (void)basm_parser_advance(parser);

    BasmToken inst_name =
        basm_parser_expect(parser, BASM_TOKEN_KIND_IDENTIFIER);

    BasmExpression expr = basm_parser_expression(parser);

    statement_out->kind = BASM_STATEMENT_KIND_INSTRUCTION;
    statement_out->u.inst =
        (BasmInst){.label = label, .name = inst_name, .expr = expr};

    if (parser->current.kind != BASM_TOKEN_KIND_END_OF_INPUT)
      basm_parser_munch(parser, BASM_TOKEN_KIND_NEW_LINE);

    return true;
  } break;
  case BASM_TOKEN_KIND_END_OF_INPUT:
    return false;
  case BASM_TOKEN_KIND_PERCENT: {
    basm_parser_munch(parser, BASM_TOKEN_KIND_PERCENT);

    const BasmToken statement_token =
        basm_parser_expect(parser, BASM_TOKEN_KIND_IDENTIFIER);

    const StringView name = statement_token.lexeme;
    if (sv_eq(name, sv_from_cstr("bind"))) {
      const BasmToken name =
          basm_parser_expect(parser, BASM_TOKEN_KIND_IDENTIFIER);

      BasmExpression expr = basm_parser_expression(parser);

      statement_out->kind = BASM_STATEMENT_KIND_BIND;
      statement_out->u.bind = (BasmBind){.name = name, .expr = expr};

      if (parser->current.kind != BASM_TOKEN_KIND_END_OF_INPUT)
        basm_parser_munch(parser, BASM_TOKEN_KIND_NEW_LINE);

      return true;
    } else if (sv_eq(name, sv_from_cstr("include"))) {
      const BasmToken path_token =
          basm_parser_expect(parser, BASM_TOKEN_KIND_STRING);
      statement_out->kind = BASM_STATEMENT_KIND_INCLUDE;
      statement_out->u.include = (BasmInclude){.path = path_token};
      return true;
    } else {
      PANIC("unknown statement '" SV_FMT "'.", SV_ARG(statement_token.lexeme));
    }

  } break;
  case BASM_TOKEN_KIND_STRING:
  case BASM_TOKEN_KIND_INTEGER:
  case BASM_TOKEN_KIND_FLOAT:
  case BASM_TOKEN_KIND_COLON:
  case BASM_TOKEN_KIND_MINUS:

  case BASM_TOKEN_KIND_NEW_LINE:
    while (parser->current.kind == BASM_TOKEN_KIND_NEW_LINE)
      (void)basm_parser_advance(parser);
    return basm_parser_statement(parser, statement_out);
  default:
    BM_UNREACHABLE();
  }
}

#endif