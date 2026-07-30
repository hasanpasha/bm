#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include "string_view.h"
#include <bm.h>

#define BASM_LABELS_CAP 1024
#define BASM_DEFERRED_OPERANDS_CAP 1024

typedef struct BASM_LABEL {
  StringView name;
  BmInstAddr addr;
} BasmLabel;

typedef struct BASM_UNRESOLVED_JMP {
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

static uint64_t basm_find_label(const Basm *basm, StringView name) {
  for (size_t i = 0; i < basm->labels_len; i++) {
    BasmLabel label = basm->labels[i];
    if (sv_eq(label.name, name)) {
      return label.addr;
    }
  }

  fprintf(stderr, "Error: failed to resolved '" SV "'.\n", SV_ARG(name));
  exit(1);
}

static void basm_push_label(Basm *basm, StringView name, BmInstAddr addr) {
  assert(basm->labels_len < BASM_LABELS_CAP);
  basm->labels[basm->labels_len++] = (BasmLabel){
      .name = name,
      .addr = addr,
  };
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

static void basm_translate_source(Basm *basm, StringView source) {
  basm->prg.len = 0;
  while (!sv_is_blank(source)) {
    assert(basm->prg.len < BM_PROGRAM_CAP);

    StringView line = sv_trim(sv_chop_by_delim(&source, '\n'));

    if (sv_is_blank(line))
      continue;

    if (sv_begins_with(line, sv_from_cstr("#")))
      continue;

    line = sv_ltrim(line);
    StringView inst_name = sv_chop_by_delim(&line, ' ');

    if (sv_ends_with(inst_name, sv_from_cstr(":"))) {
      StringView label = sv_slice(inst_name, 0, -1);

      basm_push_label(basm, label, basm->prg.len);

      line = sv_ltrim(line);
      inst_name = sv_chop_by_delim(&line, ' ');
    }

    if (sv_is_blank(inst_name))
      continue;

    StringView operand = sv_trim(sv_chop_by_delim(&line, '#'));

    BmInst inst = {0};
    if (sv_eq(inst_name, sv_from_cstr("nop"))) {
      inst.type = BM_INST_TYPE_NO_OPERATION;
    } else if (sv_eq(inst_name, sv_from_cstr("push"))) {
      inst.type = BM_INST_TYPE_PUSH;
      inst.operand.u64 = sv_parse_ulong(operand);
    } else if (sv_eq(inst_name, sv_from_cstr("dup"))) {
      inst.type = BM_INST_TYPE_DUPLICATE;
      inst.operand.u64 = sv_parse_ulong(operand);
    } else if (sv_eq(inst_name, sv_from_cstr("jmp"))) {
      inst.type = BM_INST_TYPE_JUMP;
      if (isdigit(operand.ptr[0])) {
        inst.operand.u64 = sv_parse_ulong(operand);
      } else {
        basm_push_deferred_operand(basm, basm->prg.len, operand);
      }
    } else if (sv_eq(inst_name, sv_from_cstr("jt"))) {
      inst.type = BM_INST_TYPE_JMP_IF_TRUE;
      inst.operand.u64 = sv_parse_ulong(operand);
    } else if (sv_eq(inst_name, sv_from_cstr("plus"))) {
      inst.type = BM_INST_TYPE_PLUS;
    } else if (sv_eq(inst_name, sv_from_cstr("dump"))) {
      inst.type = BM_INST_TYPE_DEBUG_PRINT;
    } else if (sv_eq(inst_name, sv_from_cstr("teq"))) {
      inst.type = BM_INST_TYPE_TEST_EQUALS;
    } else if (sv_eq(inst_name, sv_from_cstr("hlt"))) {
      inst.type = BM_INST_TYPE_HALT;
    } else {
      fprintf(stderr, "Error: unknown instruction '" SV "'\n",
              SV_ARG(inst_name));
      exit(1);
    }

    bm_program_push(&basm->prg, inst);
  }

  for (size_t i = 0; i < basm->deferred_operands_len; i++) {
    BasmDeferredOperand jmp = basm->deferred_operands[i];
    uint64_t addr = basm_find_label(basm, jmp.label);
    basm->prg.ptr[jmp.addr].operand.u64 = addr;
  }
}

static bool basm_assemble_file(Basm *basm, const char *input_path,
                               const char *output_path) {
  StringView sv = sv_read_file(input_path);
  basm_translate_source(basm, sv);
  free((void *)sv.ptr);

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
    fprintf(stderr, "Error: expected input\n");
    return EXIT_FAILURE;
  }

  const char *output_file = shift(&argc, &argv);
  if (output_file == NULL) {
    usage(stderr, program);
    fprintf(stderr, "Error: expected output\n");
    return EXIT_FAILURE;
  }

  if (!basm_assemble_file(&basm, input_file, output_file))
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}