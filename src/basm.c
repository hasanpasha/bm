#include <stdlib.h>

#include "string_view.h"
#include <bm.h>

static BmInst basm_translate_line(StringView line) {
  line = sv_ltrim(line);
  StringView inst_name = sv_chop_by_delim(&line, ' ');

  if (sv_eq(inst_name, sv_from_cstr("push"))) {
    line = sv_ltrim(line);
    BmWord operand = sv_parse_ulong(sv_rtrim(line));
    return (BmInst){.type = BM_INST_TYPE_PUSH, .operand = operand};
  } else if (sv_eq(inst_name, sv_from_cstr("dup"))) {
    line = sv_ltrim(line);
    BmWord operand = sv_parse_ulong(sv_rtrim(line));
    return (BmInst){.type = BM_INST_TYPE_DUP, .operand = operand};
  } else if (sv_eq(inst_name, sv_from_cstr("jmp"))) {
    line = sv_ltrim(line);
    BmWord operand = sv_parse_ulong(sv_rtrim(line));
    return (BmInst){.type = BM_INST_TYPE_JMP, .operand = operand};
  } else if (sv_eq(inst_name, sv_from_cstr("jnz"))) {
    line = sv_ltrim(line);
    BmWord operand = sv_parse_ulong(sv_rtrim(line));
    return (BmInst){.type = BM_INST_TYPE_JNZ, .operand = operand};
  } else if (sv_eq(inst_name, sv_from_cstr("plus"))) {
    return (BmInst){.type = BM_INST_TYPE_PLUS};
  } else if (sv_eq(inst_name, sv_from_cstr("dump"))) {
    return (BmInst){.type = BM_INST_TYPE_DUMP};
  } else if (sv_eq(inst_name, sv_from_cstr("teq"))) {
    return (BmInst){.type = BM_INST_TYPE_TEQ};
  } else if (sv_eq(inst_name, sv_from_cstr("hlt"))) {
    return (BmInst){.type = BM_INST_TYPE_HLT};
  } else {
    fprintf(stderr, "Error: unknown instruction '" SV "'\n", SV_ARG(inst_name));
    exit(1);
  }

  return (BmInst){0};
}

static size_t basm_translate_source(StringView source, BmInst *program,
                                    size_t program_cap) {

  size_t program_size = 0;
  while (source.len > 0 && program_size < program_cap) {
    StringView line = sv_trim(sv_chop_by_delim(&source, '\n'));
    if (!sv_is_blank(line)) {
      program[program_size++] = basm_translate_line(line);
    }
  }

  return program_size;
}

#define BASM_PROGRAM_CAP 1024

BmInst program[BASM_PROGRAM_CAP];

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Error: usage: %s <input_file> <output_file>\n", argv[0]);
    return EXIT_FAILURE;
  }

  char *input_file = argv[1];
  char *output_file = argv[2];

  StringView sv = read_file(input_file);
  size_t program_size = basm_translate_source(sv, program, BASM_PROGRAM_CAP);

  if (!bm_save_program_to_file(program, program_size, output_file))
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}