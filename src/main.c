#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bm.h>

Bm bm = {0};

int main() {
  if (!bm_load_program_from_file(&bm, "fib.bm"))
    return EXIT_FAILURE;

  BmError error = BM_ERROR_OK;
  size_t i = 0;
  while (!bm.halted && i < 128) {
    i++;
    // bm_dump(&bm, stdout);
    if ((error = bm_step(&bm)) != BM_ERROR_OK) {
      fprintf(stderr, "Error: %s\n", bm_error_string(error));
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}