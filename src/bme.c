#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BM_IMPLEMENTATION
#include <bm.h>

Bm bm = {0};

static char *shift(int *argc, char ***argv) {
  if (*argc < 1)
    return NULL;
  char *arg = **argv;
  *argc -= 1;
  *argv += 1;
  return arg;
}

static void usage(FILE *stream, const char *program) {
  fprintf(stream, "Usage: %s <input.bm> [-h] [-l limit]\n", program);
}

int main(int argc, char *argv[]) {
  const char *program = shift(&argc, &argv);

  char *input_file = NULL;
  int limit = -1;

  char *arg = NULL;
  while ((arg = shift(&argc, &argv)) != NULL) {
    if (arg[0] == '-') {
      char *flag = &arg[1];

      if (strncmp(flag, "h", 1) == 0) {
        usage(stdout, program);
        return EXIT_SUCCESS;
      } else if (strncmp(flag, "l", 1) == 0) {
        char *limit_arg = shift(&argc, &argv);
        if (limit_arg == NULL) {
          usage(stderr, program);
          fprintf(stderr, "Error: expected limit value.\n");
          return EXIT_FAILURE;
        }

        limit = (int)strtol(limit_arg, NULL, 10);
      } else {
        usage(stderr, program);
        fprintf(stderr, "Error: unknown flag '%s'\n", flag);
        return EXIT_FAILURE;
      }
    } else {
      if (input_file != NULL) {
        usage(stderr, program);
        fprintf(stderr, "Error: <input.bm> provided more than once.\n");
        return EXIT_FAILURE;
      }

      input_file = arg;
    }
  }

  if (input_file == NULL) {
    usage(stderr, program);
    fprintf(stderr, "Error: expected input.\n");
    return EXIT_FAILURE;
  }

  if (!bm_program_load_from_file(&bm.program, input_file)) {
    fprintf(stderr, "Error: failed to load program from '%s'.\n", input_file);
    return EXIT_FAILURE;
  }

  BmError error = bm_execute_program(&bm, limit);
  if (error != BM_ERROR_OK) {
    fprintf(stderr, "Error: %s\n", bm_error_string(error));
    bm_stack_dump(&bm.stack, stderr);
    fprintf(stderr, "\n");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}