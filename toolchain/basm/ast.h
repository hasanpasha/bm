#ifndef BASM_AST_H
#define BASM_AST_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "token.h"

#define BASM_INST_OPERANDS_CAP 32

typedef enum BASM_EXPRESSION_KIND {
  BASM_EXPRESSION_KIND_NONE,
  BASM_EXPRESSION_KIND_INTEGER,
  BASM_EXPRESSION_KIND_FLOAT,
  BASM_EXPRESSION_KIND_VARIABLE,
  BASM_EXPRESSION_KIND_UNARY,
  BASM_EXPRESSION_KIND_PC,
} BasmExpressionKind;

const char *basm_expression_kind_string(BasmExpressionKind kind);

typedef struct BASM_EXPRESSION BasmExpression;

typedef enum BASM_EXPRESSION_UNARY_OPERATOR {
  BASM_EXPRESSION_UNARY_OPERATOR_MINUS,
} BasmExpressionUnaryOperator;

const char *
    basm_expression_unary_operator_string(BasmExpressionUnaryOperator operator);

typedef struct BASM_EXPRESSION {
  BasmExpressionKind kind;
  union BASM_EXPRESSION_UNION {
    uint64_t integer;
    double _float;
    BasmToken variable;
    struct BASM_EXPRESSION_UNARY {
      BasmExpressionUnaryOperator operator;
      BasmExpression *operand;
    } unary;
  } u;
} BasmExpression;

void basm_expression_dump(const BasmExpression *expr, FILE *stream);

typedef struct BASM_INST {
  BasmToken label;
  BasmToken name;
  BasmExpression expr;
} BasmInst;

typedef struct BASM_BIND {
  BasmToken name;
  BasmExpression expr;
} BasmBind;

typedef struct BASM_INCLUDE {
  BasmToken path;
} BasmInclude;

typedef enum BASM_STATEMENT_KIND {
  BASM_STATEMENT_KIND_INSTRUCTION,
  BASM_STATEMENT_KIND_BIND,
  BASM_STATEMENT_KIND_INCLUDE,
} BasmStatementKind;

const char *basme_statement_kind_string(BasmStatementKind kind);

typedef struct BASM_STATEMENT {
  BasmStatementKind kind;
  union BASM_STATEMENT_UNION {
    BasmInst inst;
    BasmBind bind;
    BasmInclude include;
  } u;
} BasmStatement;

void basm_statement_dump(BasmStatement stmt, FILE *stream);

const char *basm_expression_kind_string(BasmExpressionKind kind) {
  switch (kind) {
  case BASM_EXPRESSION_KIND_NONE:
    return "NONE";
  case BASM_EXPRESSION_KIND_INTEGER:
    return "INTEGER";
  case BASM_EXPRESSION_KIND_FLOAT:
    return "FLOAT";
  case BASM_EXPRESSION_KIND_VARIABLE:
    return "VARIABLE";
  case BASM_EXPRESSION_KIND_UNARY:
    return "UNARY";
  case BASM_EXPRESSION_KIND_PC:
    return "PC";
  default:
    BM_UNREACHABLE();
    break;
  }
}

const char *
basm_expression_unary_operator_string(BasmExpressionUnaryOperator operator) {
  switch (operator) {
  case BASM_EXPRESSION_UNARY_OPERATOR_MINUS:
    return "MINUS";
  default:
    BM_UNREACHABLE();
  }
}

void basm_expression_dump(const BasmExpression *expr, FILE *stream) {
  fprintf(stream, "%s(", basm_expression_kind_string(expr->kind));
  switch (expr->kind) {
  case BASM_EXPRESSION_KIND_NONE:
    break;
  case BASM_EXPRESSION_KIND_INTEGER:
    fprintf(stream, "%ld", expr->u.integer);
    break;
  case BASM_EXPRESSION_KIND_FLOAT:
    fprintf(stream, "%lf", expr->u._float);
    break;
  case BASM_EXPRESSION_KIND_VARIABLE:
    fprintf(stream, SV_FMT, SV_ARG(expr->u.variable.lexeme));
    break;
  case BASM_EXPRESSION_KIND_UNARY:
    fprintf(stream, "%s, ",
            basm_expression_unary_operator_string(expr->u.unary.operator));
    basm_expression_dump(expr->u.unary.operand, stream);
    break;
  case BASM_EXPRESSION_KIND_PC:
    fprintf(stream, "$");
    break;
  default:
    BM_UNREACHABLE();
    break;
  }
  fprintf(stream, ")");
}

const char *basme_statement_kind_string(BasmStatementKind kind) {
  switch (kind) {
  case BASM_STATEMENT_KIND_INSTRUCTION:
    return "INSTRUCTION";
  case BASM_STATEMENT_KIND_BIND:
    return "BIND";
  case BASM_STATEMENT_KIND_INCLUDE:
    return "INCLUDE";
  default:
    BM_UNREACHABLE();
  }
}

void basm_statement_dump(BasmStatement stmt, FILE *stream) {
  fprintf(stream, "%s(", basme_statement_kind_string(stmt.kind));
  switch (stmt.kind) {
  case BASM_STATEMENT_KIND_INSTRUCTION: {
    BasmInst inst = stmt.u.inst;
    fprintf(stream, "label:%s('" SV_FMT "'), name:%s('" SV_FMT "'), operand:",
            basm_token_kind_string(inst.label.kind), SV_ARG(inst.label.lexeme),
            basm_token_kind_string(inst.label.kind), SV_ARG(inst.name.lexeme));
    basm_expression_dump(&inst.expr, stream);
  } break;
  case BASM_STATEMENT_KIND_BIND: {
    BasmBind bind = stmt.u.bind;
    fprintf(stream, "name:'" SV_FMT "'), expr:", SV_ARG(bind.name.lexeme));
    basm_expression_dump(&bind.expr, stream);
  } break;
  case BASM_STATEMENT_KIND_INCLUDE: {
    fprintf(stream, "'" SV_FMT "'", SV_ARG(stmt.u.include.path.lexeme));
  } break;
  default:
    BM_UNREACHABLE();
    break;
  }
  fprintf(stream, ")\n");
}

#endif
