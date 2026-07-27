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
    case BM_INST_TYPE_NO_OPERATION:
      printf("nop\n");
      break;
    case BM_INST_TYPE_HALT:
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
    case BM_INST_TYPE_MULTIPLY:
      printf("mult\n");
      break;
    case BM_INST_TYPE_DIVIDE:
      printf("div\n");
      break;
    case BM_INST_TYPE_JUMP:
      printf("jmp %zu\n", inst.operand);
      break;
    case BM_INST_TYPE_DROP:
      printf("drop\n");
      break;
    case BM_INST_TYPE_JMP_IF_TRUE:
      printf("jt %zu\n", inst.operand);
      break;
    case BM_INST_TYPE_TEST_EQUALS:
      printf("teq\n");
      break;
    case BM_INST_TYPE_DEBUG_PRINT:
      printf("dump\n");
      break;
    case BM_INST_TYPE_DUPLICATE:
      printf("dup %zu\n", inst.operand);
      break;
    }
  }

  return EXIT_SUCCESS;
}
