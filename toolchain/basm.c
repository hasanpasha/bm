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
#include "basm/token.h" fprint

#define BASM_LABELS_CAP 1024
#define BASM_DEFERRED_OPERANDS_CAP 1024

typedef struct BASM_LABEL {
  StringView name;
  BasmExpression value;
} BasmLabel;

typedef struct BASM_DEFERRED_OPERAND {
  BmInstAddr addr;
  StringView label;
} BasmDeferredOperand;

typedef struct BASM {
  BmProgram prg;
  BasmLabel labels[BASM_LABELS_CAP];
  size_t labels_len;
  BasmDeferredOperand deferred_operands[BASM_DEFERRED_OPERANDS_CAP];
  size_t deferred_operands_len;
} Basm;

static bool basm_find_label(const Basm *basm, StringView name,
                            BasmExpression *value) {
  for (size_t i = 0; i < basm->labels_len; i++) {
    BasmLabel label = basm->labels[i];
    if (sv_eq(label.name, name)) {
      if (value != NULL) {
        *value = label.value;
      }
      return true;
    }
  }

  return false;
}

static void basm_push_value(Basm *basm, StringView name, BasmExpression value) {
  assert(basm->labels_len < BASM_LABELS_CAP);
  basm->labels[basm->labels_len++] = (BasmLabel){.name = name, .value = value};
}

static void basm_push_deferred_operand(Basm *basm, BmInstAddr addr,
                                       StringView label) {
  assert(basm->deferred_operands_len < BASM_DEFERRED_OPERANDS_CAP);
  basm->deferred_operands[basm->deferred_operands_len++] =
      (BasmDeferredOperand){
          .addr = addr,
          .label = label,
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
    if (basm_find_label(basm, expr.u.variable.lexeme, &value)) {
      expr = value;
    } else {
      basm_push_deferred_operand(basm, basm->prg.len, expr.u.variable.lexeme);
    }
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

static BmWord basm_parse_word(Basm *basm, BasmExpression expr, bool force) {
  (void)basm;

  expr = basm_fold_expr(basm, expr);

  BmWord word = {0};

  switch (expr.kind) {
  case BASM_EXPRESSION_KIND_NONE:
    PANIC("can't resolve none expression");
    break;
  case BASM_EXPRESSION_KIND_INTEGER:
    word.u64 = expr.u.integer;
    break;
  case BASM_EXPRESSION_KIND_FLOAT:
    word.f64 = expr.u._float;
    break;
  case BASM_EXPRESSION_KIND_VARIABLE:
  case BASM_EXPRESSION_KIND_UNARY:
    if (force)
      PANIC("can't resolve expr");
    break;
  default:
    BM_UNREACHABLE();
  }

  return word;
}

#define TRY_MATCH_NO_OPERAND_INST(inst_type)                                   \
  if (sv_eq(basm_inst.inst_name.lexeme,                                        \
            sv_from_cstr(bm_inst_type_readable_name(inst_type))))              \
    inst.type = inst_type;

static bool basm_assemble_file(Basm *basm, const char *input_path,
                               const char *output_path) {
  StringView source = sv_read_file(input_path);

  BasmLexer lexer = basm_lexer_init(source);
  BasmParser parser = basm_parser_init(lexer);

  BasmStatement stmt;
  while (basm_parser_statement(&parser, &stmt)) {
    if (stmt.kind != BASM_STATEMENT_KIND_INSTRUCTION)
      PANIC("only instruction statement are supported at the moment");

    BasmInst basm_inst = stmt.u.inst;

    if (!sv_is_blank(basm_inst.label.lexeme)) {
      basm_push_value(basm, basm_inst.label.lexeme,
                      (BasmExpression){.kind = BASM_EXPRESSION_KIND_INTEGER,
                                       .u.integer = basm->prg.len});
    }

    BmInst inst = {0};
    bool found = false;
    for (size_t i = 0; i < BM_NUM_OF_INST_TYPES; i++) {
      BmInstType type = (BmInstType)i;

      if (!bm_inst_type_has_operand(type) &&
          sv_eq(basm_inst.inst_name.lexeme,
                sv_from_cstr(bm_inst_type_readable_name(type)))) {
        inst.type = type;
        found = true;
        break;
      }
    }

    if (found) {
      bm_program_push(&basm->prg, inst);
      continue;
    }

    if (sv_eq(sv_from_cstr(bm_inst_type_readable_name(BM_INST_TYPE_PUSH)),
              basm_inst.inst_name.lexeme)) {
      inst.type = BM_INST_TYPE_PUSH;
      inst.operand = basm_parse_word(basm, basm_inst.expr, false);
    } else if (sv_eq(
                   sv_from_cstr(bm_inst_type_readable_name(BM_INST_TYPE_JUMP)),
                   basm_inst.inst_name.lexeme)) {
      inst.type = BM_INST_TYPE_JUMP;
      inst.operand = basm_parse_word(basm, basm_inst.expr, false);
    } else if (sv_eq(sv_from_cstr(
                         bm_inst_type_readable_name(BM_INST_TYPE_JMP_IF_TRUE)),
                     basm_inst.inst_name.lexeme)) {
      inst.type = BM_INST_TYPE_JMP_IF_TRUE;
      inst.operand = basm_parse_word(basm, basm_inst.expr, false);
    } else if (sv_eq(
                   sv_from_cstr(bm_inst_type_readable_name(BM_INST_TYPE_CALL)),
                   basm_inst.inst_name.lexeme)) {
      inst.type = BM_INST_TYPE_CALL;
      inst.operand = basm_parse_word(basm, basm_inst.expr, false);
    } else if (sv_eq(sv_from_cstr(
                         bm_inst_type_readable_name(BM_INST_TYPE_DUPLICATE)),
                     basm_inst.inst_name.lexeme)) {
      inst.type = BM_INST_TYPE_DUPLICATE;
      inst.operand = basm_parse_word(basm, basm_inst.expr, false);
    } else if (sv_eq(
                   sv_from_cstr(bm_inst_type_readable_name(BM_INST_TYPE_SWAP)),
                   basm_inst.inst_name.lexeme)) {
      inst.type = BM_INST_TYPE_SWAP;
      inst.operand = basm_parse_word(basm, basm_inst.expr, false);
    } else if (sv_eq(sv_from_cstr(
                         bm_inst_type_readable_name(BM_INST_TYPE_NATIVE)),
                     basm_inst.inst_name.lexeme)) {
      inst.type = BM_INST_TYPE_NATIVE;
      inst.operand = basm_parse_word(basm, basm_inst.expr, false);
    } else {
      PANIC("unknown inst name '" SV_FMT "'",
            SV_ARG(basm_inst.inst_name.lexeme));
    }

    bm_program_push(&basm->prg, inst);
  }

  for (size_t i = 0; i < basm->deferred_operands_len; i++) {
    BasmDeferredOperand jmp = basm->deferred_operands[i];
    BasmExpression value;
    if (!basm_find_label(basm, jmp.label, &value))
      PANIC("failed to resolve '" SV_FMT "'.", SV_ARG(jmp.label));
    basm->prg.ptr[jmp.addr].operand = basm_parse_word(basm, value, true);
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