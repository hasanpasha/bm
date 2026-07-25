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

  BmError error;
  if ((error = bm_execute_program(&bm, -1)) != BM_ERROR_OK) {
    fprintf(stderr, "Error: %s\n", bm_error_string(error));
    bm_stack_dump(&bm, stderr);
    fprintf(stderr, "\n");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}