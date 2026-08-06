#include <stdio.h>
#include <stdlib.h>

#define BM_IMPLEMENTATION
#include <bm.h>

BmProgram prg = {0};

int main(int argc, char const *argv[]) {
  if (argc < 2)
    PANIC("usage: %s <input.bm>", argv[0]);

  const char *input_file = argv[1];

  if (!bm_program_load_from_file(&prg, input_file))
    PANIC("Error: failed to load program from file '%s'.", input_file);

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
