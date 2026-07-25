#include <stdlib.h>

#include "string_view.h"
#include <bm.h>

static BmInst basm_translate_line(StringView line) {
  line = sv_ltrim(line);
  StringView inst_name = sv_chop_by_delim(&line, ' ');
  StringView operand = sv_trim(sv_chop_by_delim(&line, '#'));

  BmInst inst = {0};

  if (sv_eq(inst_name, sv_from_cstr("push"))) {
    inst =
        (BmInst){.type = BM_INST_TYPE_PUSH, .operand = sv_parse_ulong(operand)};
  } else if (sv_eq(inst_name, sv_from_cstr("dup"))) {
    inst =
        (BmInst){.type = BM_INST_TYPE_DUP, .operand = sv_parse_ulong(operand)};
  } else if (sv_eq(inst_name, sv_from_cstr("jmp"))) {
    inst =
        (BmInst){.type = BM_INST_TYPE_JMP, .operand = sv_parse_ulong(operand)};
  } else if (sv_eq(inst_name, sv_from_cstr("jt"))) {
    inst =
        (BmInst){.type = BM_INST_TYPE_JT, .operand = sv_parse_ulong(operand)};
  } else if (sv_eq(inst_name, sv_from_cstr("plus"))) {
    inst = (BmInst){.type = BM_INST_TYPE_PLUS};
  } else if (sv_eq(inst_name, sv_from_cstr("dump"))) {
    inst = (BmInst){.type = BM_INST_TYPE_DUMP};
  } else if (sv_eq(inst_name, sv_from_cstr("teq"))) {
    inst = (BmInst){.type = BM_INST_TYPE_TEQ};
  } else if (sv_eq(inst_name, sv_from_cstr("hlt"))) {
    inst = (BmInst){.type = BM_INST_TYPE_HLT};
  } else {
    fprintf(stderr, "Error: unknown instruction '" SV "'\n", SV_ARG(inst_name));
    exit(1);
  }

  return inst;
}

static size_t basm_translate_source(StringView source, BmInst *program,
                                    size_t program_cap) {

  size_t program_size = 0;
  while (!sv_is_blank(source) && program_size < program_cap) {
    StringView line = sv_trim(sv_chop_by_delim(&source, '\n'));
    if (!sv_is_blank(line)) {
      if (sv_begins_with(line, sv_from_cstr("#"))) {
        continue;
      } else {
        program[program_size++] = basm_translate_line(line);
      }
    }
  }

  return program_size;
}

Bm bm = {0};

char *shift(int *argc, char ***argv) {
  if (*argc < 1)
    return NULL;
  char *arg = **argv;
  *argc -= 1;
  *argv += 1;
  return arg;
}

void usage(FILE *stream, const char *program) {
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

  StringView sv = sv_read_file(input_file);
  bm.program_size = basm_translate_source(sv, bm.program, BM_PROGRAM_CAP);
  free((void *)sv.ptr);

  if (!bm_save_program_to_file(&bm, output_file))
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}