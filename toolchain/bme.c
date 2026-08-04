#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BM_IMPLEMENTATION
#include <bm.h>

Bm bm = {0};

static BmError bm_alloc(Bm *bm) {
  BmWord size_arg;
  BM_CATCH_ERROR(bm_pop_word(bm, &size_arg));

  const BmWord ptr = {.ptr = malloc(size_arg.u64)};
  BM_CATCH_ERROR(bm_push_word(bm, ptr));

  char *buffer = (char *)ptr.ptr;
  buffer[10] = 'a';

  return BM_ERROR_OK;
}

static BmError bm_free(Bm *bm) {
  BmWord ptr_arg;
  BM_CATCH_ERROR(bm_pop_word(bm, &ptr_arg));

  free(ptr_arg.ptr);

  return BM_ERROR_OK;
}

static char *shift(int *argc, char ***argv) {
  if (*argc < 1)
    return NULL;
  char *arg = **argv;
  *argc -= 1;
  *argv += 1;
  return arg;
}

static void usage(FILE *stream, const char *program) {
  fprintf(stream, "Usage: %s <input.bm> [-h] [-l limit] [-d]\n", program);
}

int main(int argc, char *argv[]) {
  const char *program = shift(&argc, &argv);

  char *input_file = NULL;
  int limit = -1;
  bool debug = false;

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
      } else if (strncmp(flag, "d", 1) == 0) {
        debug = true;
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

  if (!bm_push_native_func(&bm, bm_alloc)) {
    fprintf(stderr, "Error: failed to add native function.\n");
    return EXIT_FAILURE;
  }

  if (!bm_push_native_func(&bm, bm_free)) {
    fprintf(stderr, "Error: failed to add native function.\n");
    return EXIT_FAILURE;
  }

  if (!debug) {
    BmError error = bm_execute_program(&bm, limit);
    if (error != BM_ERROR_OK) {
      fprintf(stderr, "Error: %s\n", bm_error_string(error));
      bm_stack_dump(&bm.stack, stderr);
      return EXIT_FAILURE;
    }
  } else {
    BmError error = BM_ERROR_OK;
    BmInst inst = {0};

    while (!bm.halted && limit != 0) {

      if ((error = bm_pop_inst(&bm, &inst)) != BM_ERROR_OK) {
        fprintf(stderr, "Error: %s\n", bm_error_string(error));
        return EXIT_FAILURE;
      }
      bm_inst_dump(inst, stdout);
      printf("\n");
      bm_stack_dump(&bm.stack, stdout);

      (void)fgetc(stdin);
      if ((error = bm_execute_inst(&bm, inst)) != BM_ERROR_OK) {
        fprintf(stderr, "Error: %s\n", bm_error_string(error));
        bm_stack_dump(&bm.stack, stderr);
        return EXIT_FAILURE;
      }

      if (limit > 0)
        limit--;
    }
  }

  return EXIT_SUCCESS;
}