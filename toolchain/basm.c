#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#define BM_IMPLEMENTATION
#include <bm.h>

#define STRING_VIEW_IMPLEMENTATION
#include <string_view.h>

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

static BmWord basm_parse_word(StringView sv) {
  sv = sv_trim(sv);
  BmWord word;
  if (sv_chop_right(&sv, sv_from_cstr("u64"))) {
    word.u64 = sv_parse_ulong(sv);
  } else if (sv_chop_right(&sv, sv_from_cstr("i64"))) {
    word.i64 = sv_parse_long(sv);
  } else if (sv_chop_right(&sv, sv_from_cstr("f64"))) {
    word.f64 = sv_parse_double(sv);
  } else {
    word.u64 = sv_parse_ulong(sv);
  }
  return word;
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

    if (sv_chop_right(&inst_name, sv_from_cstr(":"))) {
      basm_push_label(basm, inst_name, basm->prg.len);

      line = sv_ltrim(line);
      inst_name = sv_chop_by_delim(&line, ' ');
    }

    if (sv_is_blank(inst_name))
      continue;

    StringView operand = sv_trim(sv_chop_by_delim(&line, '#'));

#define TRY_PARSE_NO_OP_INST(bm_type)                                          \
  if (sv_eq(inst_name, sv_from_cstr(bm_inst_type_readable_name(bm_type))))     \
    inst.type = bm_type;

    BmInst inst = {0};
    TRY_PARSE_NO_OP_INST(BM_INST_TYPE_NO_OPERATION)
    else if (sv_eq(inst_name, sv_from_cstr("push"))) {
      inst.type = BM_INST_TYPE_PUSH;
      inst.operand = basm_parse_word(operand);
    }
    else if (sv_eq(inst_name, sv_from_cstr("dup"))) {
      inst.type = BM_INST_TYPE_DUPLICATE;
      inst.operand.u64 = sv_parse_ulong(operand);
    }
    else if (sv_eq(inst_name, sv_from_cstr("jmp"))) {
      inst.type = BM_INST_TYPE_JUMP;
      if (isdigit(operand.ptr[0])) {
        inst.operand.u64 = sv_parse_ulong(operand);
      } else {
        basm_push_deferred_operand(basm, basm->prg.len, operand);
      }
    }
    else if (sv_eq(inst_name, sv_from_cstr("call"))) {
      inst.type = BM_INST_TYPE_CALL;
      if (isdigit(operand.ptr[0])) {
        inst.operand.u64 = sv_parse_ulong(operand);
      } else {
        basm_push_deferred_operand(basm, basm->prg.len, operand);
      }
    }
    else if (sv_eq(inst_name, sv_from_cstr("jt"))) {
      inst.type = BM_INST_TYPE_JMP_IF_TRUE;
      if (isdigit(operand.ptr[0])) {
        inst.operand.u64 = sv_parse_ulong(operand);
      } else {
        basm_push_deferred_operand(basm, basm->prg.len, operand);
      }
    }
    else if (sv_eq(inst_name, sv_from_cstr("swap"))) {
      inst.type = BM_INST_TYPE_SWAP;
      inst.operand.u64 = sv_parse_ulong(operand);
    }
    else TRY_PARSE_NO_OP_INST(BM_INST_TYPE_DROP)                          //
        else TRY_PARSE_NO_OP_INST(BM_INST_TYPE_PLUS_INT)                  //
        else TRY_PARSE_NO_OP_INST(BM_INST_TYPE_MINUS_INT)                 //
        else TRY_PARSE_NO_OP_INST(BM_INST_TYPE_MULTIPLY_INT)              //
        else TRY_PARSE_NO_OP_INST(BM_INST_TYPE_DIVIDE_INT)                //
        else TRY_PARSE_NO_OP_INST(BM_INST_TYPE_PLUS_FLOAT)                //
        else TRY_PARSE_NO_OP_INST(BM_INST_TYPE_MINUS_FLOAT)               //
        else TRY_PARSE_NO_OP_INST(BM_INST_TYPE_MULTIPLY_FLOAT)            //
        else TRY_PARSE_NO_OP_INST(BM_INST_TYPE_DIVIDE_FLOAT)              //
        else TRY_PARSE_NO_OP_INST(BM_INST_TYPE_DEBUG_PRINT)               //
        else TRY_PARSE_NO_OP_INST(BM_INST_TYPE_TEST_EQUALS)               //
        else TRY_PARSE_NO_OP_INST(BM_INST_TYPE_HALT)                      //
        else TRY_PARSE_NO_OP_INST(BM_INST_TYPE_NOT)                       //
        else TRY_PARSE_NO_OP_INST(BM_INST_TYPE_TEST_GREATER_EQUALS_FLOAT) //
        else TRY_PARSE_NO_OP_INST(BM_INST_TYPE_RETURN)                    //
        else {
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

#undef TRY_PARSE_NO_OP_INST
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