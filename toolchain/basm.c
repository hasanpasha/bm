#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#define BM_IMPLEMENTATION
#include <bm.h>

#define STRING_VIEW_IMPLEMENTATION
#include <string_view.h>

#include "basm/ast.h"
#include "basm/lexer.h"
#include "basm/parser.h"
#include "basm/token.h"

#define BASM_VARIABLES_CAP 1024
#define BASM_DEFERRED_OPERANDS_CAP 1024

typedef struct BASM_VARIABLE {
  StringView name;
  BasmExpression value;
} BasmVariable;

typedef struct BASM_DEFERRED_OPERAND {
  BmInstAddr addr;
  BasmExpression expr;
} BasmDeferredOperand;

typedef struct BASM {
  BmProgram prg;
  BasmVariable variables[BASM_VARIABLES_CAP];
  size_t variables_len;
  BasmDeferredOperand deferred_operands[BASM_DEFERRED_OPERANDS_CAP];
  size_t deferred_operands_len;
} Basm;

static bool basm_get_variable_value(const Basm *basm, StringView name,
                                    BasmExpression *value) {
  for (size_t i = 0; i < basm->variables_len; i++) {
    BasmVariable label = basm->variables[i];
    if (sv_eq(label.name, name)) {
      if (value != NULL) {
        *value = label.value;
      }
      return true;
    }
  }

  return false;
}

static void basm_push_variable(Basm *basm, StringView name,
                               BasmExpression value) {
  assert(basm->variables_len < BASM_VARIABLES_CAP);
  basm->variables[basm->variables_len++] =
      (BasmVariable){.name = name, .value = value};
}

static void basm_push_deferred_operand(Basm *basm, BmInstAddr addr,
                                       BasmExpression label) {
  assert(basm->deferred_operands_len < BASM_DEFERRED_OPERANDS_CAP);
  basm->deferred_operands[basm->deferred_operands_len++] =
      (BasmDeferredOperand){
          .addr = addr,
          .expr = label,
      };
}

static BasmExpression basm_fold_expr(Basm *basm, BasmExpression expr) {
  switch (expr.kind) {
  case BASM_EXPRESSION_KIND_NONE:
  case BASM_EXPRESSION_KIND_INTEGER:
  case BASM_EXPRESSION_KIND_FLOAT:
    break;
  case BASM_EXPRESSION_KIND_VARIABLE: {
    BasmExpression value;
    if (basm_get_variable_value(basm, expr.u.variable.lexeme, &value))
      expr = value;
  } break;
  case BASM_EXPRESSION_KIND_UNARY: {
    BasmExpression operand_value = basm_fold_expr(basm, *expr.u.unary.operand);
    switch (operand_value.kind) {
    case BASM_EXPRESSION_KIND_INTEGER: {
      expr.kind = operand_value.kind;
      switch (expr.u.unary.operator) {
      case BASM_EXPRESSION_UNARY_OPERATOR_MINUS:
        expr.u.integer = -operand_value.u.integer;
        break;
      default:
        break;
      }
    } break;
    case BASM_EXPRESSION_KIND_FLOAT: {
      expr.kind = operand_value.kind;
      switch (expr.u.unary.operator) {
      case BASM_EXPRESSION_UNARY_OPERATOR_MINUS:
        expr.u._float = -operand_value.u._float;
        break;
      default:
        break;
      }
    } break;
    case BASM_EXPRESSION_KIND_NONE:
    case BASM_EXPRESSION_KIND_VARIABLE:
    case BASM_EXPRESSION_KIND_UNARY:
      *expr.u.unary.operand = operand_value;
      break;
    default:
      BM_UNREACHABLE();
    }
  } break;
  default:
    break;
  }

  return expr;
}

static bool basm_expression_to_word(Basm *basm, BasmExpression expr,
                                    BmWord *word_out) {
  expr = basm_fold_expr(basm, expr);

  switch (expr.kind) {
  case BASM_EXPRESSION_KIND_NONE:
    PANIC("can't convert 'none' expression to 'BmWord'");
    break;
  case BASM_EXPRESSION_KIND_INTEGER:
    word_out->u64 = expr.u.integer;
    return true;
  case BASM_EXPRESSION_KIND_FLOAT:
    word_out->f64 = expr.u._float;
    return true;
  case BASM_EXPRESSION_KIND_VARIABLE:
  case BASM_EXPRESSION_KIND_UNARY:
    return false;
  default:
    BM_UNREACHABLE();
  }
}

static bool basm_assemble_file(Basm *basm, const char *input_path,
                               const char *output_path) {
  StringView source = sv_read_file(input_path);

  BasmLexer lexer = basm_lexer_init(source);
  BasmParser parser = basm_parser_init(lexer);

  BasmStatement stmt;
  while (basm_parser_statement(&parser, &stmt)) {
    if (stmt.kind == BASM_STATEMENT_KIND_BIND) {
      BasmBind bind = stmt.u.bind;

      basm_push_variable(basm, bind.name.lexeme, bind.expr);

      continue;
    }

    if (stmt.kind != BASM_STATEMENT_KIND_INSTRUCTION)
      PANIC("only instruction statement are supported at the moment");

    BasmInst basm_inst = stmt.u.inst;

    if (!sv_is_blank(basm_inst.label.lexeme)) {
      basm_push_variable(basm, basm_inst.label.lexeme,
                         (BasmExpression){.kind = BASM_EXPRESSION_KIND_INTEGER,
                                          .u.integer = basm->prg.len});
    }

    BmInst inst = {0};
    bool found = false;

    for (size_t i = 0; i < BM_NUM_OF_INST_TYPES; i++) {
      BmInstType type = (BmInstType)i;

      const char *type_name = bm_inst_type_string(type);
      if (sv_eq(basm_inst.name.lexeme, sv_from_cstr(type_name))) {
        found = true;

        inst.type = type;
        if (bm_inst_type_has_operand(type)) {
          BmWord operand = {0};
          if (!basm_expression_to_word(basm, basm_inst.expr, &operand)) {
            basm_push_deferred_operand(basm, basm->prg.len, basm_inst.expr);
          }

          inst.operand = operand;
        } else if (basm_inst.expr.kind != BASM_EXPRESSION_KIND_NONE) {
          PANIC("'%s' doesn't inst accept an operand.", type_name);
        }
      }
    }

    if (!found)
      PANIC("unknown inst name '" SV_FMT "'", SV_ARG(basm_inst.name.lexeme));

    bm_program_push(&basm->prg, inst);
  }

  for (size_t i = 0; i < basm->deferred_operands_len; i++) {
    BasmDeferredOperand deferred_operand = basm->deferred_operands[i];

    if (!basm_expression_to_word(
            basm, deferred_operand.expr,
            &basm->prg.ptr[deferred_operand.addr].operand)) {
      PANIC("failed to resolve");
    }
  }

  free((void *)source.ptr);
  return bm_program_save_to_file(&basm->prg, output_path);
}

Basm basm = {0};

static char *shift(int *argc, char ***argv) {
  if (*argc < 1)
    return NULL;
  char *arg = **argv;
  *argc -= 1;
  *argv += 1;
  return arg;
}

static void usage(FILE *stream, const char *program) {
  fprintf(stream, "Usage: %s <input.basm> <output.bm>\n", program);
}

int main(int argc, char *argv[]) {
  const char *program = shift(&argc, &argv);

  const char *input_file = shift(&argc, &argv);
  if (input_file == NULL) {
    usage(stderr, program);
    PANIC("expected input");
  }

  const char *output_file = shift(&argc, &argv);
  if (output_file == NULL) {
    usage(stderr, program);
    PANIC("expected output");
  }

  if (!basm_assemble_file(&basm, input_file, output_file))
    PANIC("failed to assemble '%s'", input_file);

  return EXIT_SUCCESS;
}