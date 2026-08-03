#include <stdio.h>
#include <stdlib.h>

#define BM_IMPLEMENTATION
#include <bm.h>

BmProgram prg = {0};

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Error: usage: %s <input.bm>\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char *input_file = argv[1];

  if (!bm_program_load_from_file(&prg, input_file)) {
    fprintf(stderr, "Error: failed to load program from file '%s'.\n",
            input_file);
    return EXIT_FAILURE;
  }

  for (uint64_t i = 0; i < prg.len; i++) {
    BmInst inst = prg.ptr[i];

    printf("%s", bm_inst_type_readable_name(inst.type));
    if (bm_inst_type_has_operand(inst.type)) {
      printf(" %ld", inst.operand.i64);
    }
    putchar('\n');
  }

  return EXIT_SUCCESS;
}
