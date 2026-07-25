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
    printf("%04ld: ", i);
    bm_inst_dump(inst, stdout);
    printf("\n");
  }

  return EXIT_SUCCESS;
}
