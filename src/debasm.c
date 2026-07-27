#include <stdio.h>
#include <stdlib.h>

#include <bm.h>

Bm bm = {0};

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Error: usage: %s <input.bm>\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char *input_file = argv[1];

  if (!bm_load_program_from_file(&bm, input_file)) {
    fprintf(stderr, "Error: failed to load program from file '%s'.\n",
            input_file);
    return EXIT_FAILURE;
  }

  for (BmWord i = 0; i < bm.program_size; i++) {
    BmInst inst = bm.program[i];
    switch (inst.type) {
    case BM_INST_TYPE_NOP:
      printf("nop\n");
      break;
    case BM_INST_TYPE_HLT:
      printf("hlt\n");
      break;
    case BM_INST_TYPE_PUSH:
      printf("push %zu\n", inst.operand);
      break;
    case BM_INST_TYPE_PLUS:
      printf("plus\n");
      break;
    case BM_INST_TYPE_MINUS:
      printf("minus\n");
      break;
    case BM_INST_TYPE_MULT:
      printf("mult\n");
      break;
    case BM_INST_TYPE_DIV:
      printf("div\n");
      break;
    case BM_INST_TYPE_JMP:
      printf("jmp %zu\n", inst.operand);
      break;
    case BM_INST_TYPE_DROP:
      printf("drop\n");
      break;
    case BM_INST_TYPE_JT:
      printf("jt %zu\n", inst.operand);
      break;
    case BM_INST_TYPE_TEQ:
      printf("teq\n");
      break;
    case BM_INST_TYPE_DUMP:
      printf("dump\n");
      break;
    case BM_INST_TYPE_DUP:
      printf("dup %zu\n", inst.operand);
      break;
    }
  }

  return EXIT_SUCCESS;
}
