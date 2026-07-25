#include <stdio.h>
#include <stdlib.h>

#include <bm.h>

Bm bm = {0};

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Error: usage: %s <input_file>\n", argv[0]);
    return EXIT_FAILURE;
  }

  char *input_file = argv[1];
  if (!bm_load_program_from_file(&bm, input_file)) {
    fprintf(stderr, "Error: failed to load program from '%s'.\n", input_file);
    return EXIT_FAILURE;
  }

  BmError error = BM_ERROR_OK;
  while (!bm.halted) {
    // bm_dump(&bm, stdout);
    if ((error = bm_step(&bm)) != BM_ERROR_OK) {
      fprintf(stderr, "Error: %s\n", bm_error_string(error));
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}