#include <bm.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

const char *bm_error_string(BmError error) {
  switch (error) {
  case BM_ERROR_OK:
    return "OK";
  case BM_ERROR_STACK_OVERFLOW:
    return "STACK_OVERFLOW";
  case BM_ERROR_STACK_UNDERFLOW:
    return "STACK_UNDERFLOW";
  case BM_ERROR_ILLEGAL_INST:
    return "ILLEGAL_INST";
  case BM_ERROR_DIVIDE_BY_ZERO:
    return "DIVIDE_BY_ZERO";
  case BM_ERROR_ILLEGAL_INST_ACCESS:
    return "ILLEGAL_INST_ACCESS";
  case BM_ERROR_ILLEGAL_OPERAND:
    return "ILLEGAL_OPERAND";
  default:
    BM_UNREACHABLE();
  }
}

const char *bm_inst_type_string(BmInstType inst_type) {
  switch (inst_type) {
  case BM_INST_TYPE_HALT:
    return "HALT";
  case BM_INST_TYPE_PUSH:
    return "PUSH";
  case BM_INST_TYPE_PLUS:
    return "PLUS";
  case BM_INST_TYPE_MINUS:
    return "MINUS";
  case BM_INST_TYPE_MULTIPLY:
    return "MULTIPLY";
  case BM_INST_TYPE_DIVIDE:
    return "DIVIDE";
  case BM_INST_TYPE_JUMP:
    return "JUMP";
  case BM_INST_TYPE_DROP:
    return "DROP";
  case BM_INST_TYPE_JMP_IF_TRUE:
    return "JMP_IF_TRUE";
  case BM_INST_TYPE_TEST_EQUALS:
    return "TEST_EQUALS";
  case BM_INST_TYPE_DEBUG_PRINT:
    return "DEBUG_PRINT";
  case BM_INST_TYPE_DUPLICATE:
    return "DUPLICATE";
  case BM_INST_TYPE_NO_OPERATION:
    return "NO_OPERATION";
  default:
    BM_UNREACHABLE();
  }
}

void bm_inst_dump(BmInst inst, FILE *stream) {
  fprintf(stream, "INST_%s", bm_inst_type_string(inst.type));
  if (inst.type == BM_INST_TYPE_PUSH || inst.type == BM_INST_TYPE_JUMP ||
      inst.type == BM_INST_TYPE_DUPLICATE) {
    fprintf(stream, "(%ld)", inst.operand);
  }
}

void bm_dump(const Bm *bm, FILE *stream) {
  BmInst inst = bm->program[bm->pc];
  fprintf(stream, "%04ld:\t", bm->pc);
  bm_inst_dump(inst, stream);
  fprintf(stream, "\t");
  bm_stack_dump(bm, stream);
  fprintf(stream, "\n");
}

void bm_stack_dump(const Bm *bm, FILE *stream) {
  fprintf(stream, "stack: ");
  for (size_t i = 0; i < bm->stack_index; i++)
    fprintf(stream, "%ld ", bm->stack[i]);
}

/******************************** Utils ********************************/

void bm_load_program_from_memory(Bm *bm, BmInst *program, BmWord program_size) {
  assert(program_size < BM_PROGRAM_CAP);
  memcpy(bm->program, program, program_size * sizeof(BmInst));
  bm->program_size = program_size;
}

bool bm_load_program_from_file(Bm *bm, const char *file_path) {

  FILE *f = fopen(file_path, "rb");
  if (f == NULL) {
    fprintf(stderr, "Error: could not open file '%s': %s\n", file_path,
            strerror(errno));
    return false;
  }

  if (fseek(f, 0, SEEK_END) < 0) {
    fprintf(stderr, "Error: could not seek to the end of file '%s': %s.\n",
            file_path, strerror(errno));
    return false;
  }

  long pos = ftell(f);
  if (pos < 0) {
    fprintf(stderr, "Error: could get size of file '%s': %s.\n", file_path,
            strerror(errno));
    return false;
  }

  size_t file_size = (size_t)pos;
  size_t program_size = file_size / sizeof(BmInst);

  if (program_size >= (BM_PROGRAM_CAP)) {
    fprintf(stderr, "Error: program is too big: %ld\n", program_size);
    return false;
  }

  if (fseek(f, 0, SEEK_SET) < 0) {
    fprintf(stderr,
            "Error: could not seek to the beginning of file '%s': %s.\n",
            file_path, strerror(errno));
    return false;
  }

  size_t read_items = fread(bm->program, sizeof(BmInst), program_size, f);
  if (read_items < program_size) {
    fprintf(stderr,
            "Error: could not load the entire program file '%s' of size %ld.\n",
            file_path, program_size);
    return false;
  }

  if (ferror(f)) {
    fprintf(stderr, "Error: could not read from file '%s': %s.\n", file_path,
            strerror(errno));
    return false;
  }

  bm->program_size = program_size;

  fclose(f);

  return true;
}

bool bm_save_program_to_file(const Bm *bm, const char *file_path) {
  FILE *f = fopen(file_path, "wb");
  if (f == NULL) {
    fprintf(stderr, "Error: could not open file '%s': %s\n", file_path,
            strerror(errno));
    return false;
  }

  fwrite(bm->program, sizeof(BmInst), bm->program_size, f);
  if (ferror(f)) {
    fprintf(stderr, "Error: could not write to file '%s': %s.\n", file_path,
            strerror(errno));
    return false;
  }

  fclose(f);

  return true;
}

/******************************** Core ********************************/

BmError bm_push(Bm *bm, BmWord operand) {
  if (bm->stack_index >= BM_STACK_CAP)
    return BM_ERROR_STACK_OVERFLOW;

  bm->stack[bm->stack_index++] = operand;
  return BM_ERROR_OK;
}

BmError bm_pop(Bm *bm, BmWord *operand_out) {
  if (bm->stack_index == 0)
    return BM_ERROR_STACK_UNDERFLOW;

  BmWord operand = bm->stack[--bm->stack_index];

  if (operand_out != NULL)
    *operand_out = operand;

  return BM_ERROR_OK;
}

BmError bm_fetch_inst(Bm *bm, BmInst *inst_out) {
  if (bm->pc >= bm->program_size)
    return BM_ERROR_ILLEGAL_INST_ACCESS;

  BmInst inst = bm->program[bm->pc++];
  if (inst_out != NULL)
    *inst_out = inst;

  return BM_ERROR_OK;
}

BmError bm_execute_inst(Bm *bm, BmInst inst) {
  BmError error = BM_ERROR_OK;

  switch (inst.type) {
  case BM_INST_TYPE_PUSH:
    if ((error = bm_push(bm, inst.operand)) != BM_ERROR_OK)
      return error;
    break;
  case BM_INST_TYPE_PLUS:
  case BM_INST_TYPE_MINUS:
  case BM_INST_TYPE_MULTIPLY:
  case BM_INST_TYPE_DIVIDE:
  case BM_INST_TYPE_TEST_EQUALS: {
    BmWord a, b, result;
    if ((error = bm_pop(bm, &b)) != BM_ERROR_OK)
      return error;
    if ((error = bm_pop(bm, &a)) != BM_ERROR_OK)
      return error;

    if (inst.type == BM_INST_TYPE_PLUS) {
      result = a + b;
    } else if (inst.type == BM_INST_TYPE_MINUS) {
      result = a - b;
    } else if (inst.type == BM_INST_TYPE_MULTIPLY) {
      result = a * b;
    } else if (inst.type == BM_INST_TYPE_DIVIDE) {
      if (b == 0)
        return BM_ERROR_DIVIDE_BY_ZERO;
      result = a / b;
    } else if (inst.type == BM_INST_TYPE_TEST_EQUALS) {
      result = a == b;
    }

    if ((error = bm_push(bm, result)) != BM_ERROR_OK)
      return error;
  } break;
  case BM_INST_TYPE_JUMP:
    bm->pc = inst.operand;
    break;
  case BM_INST_TYPE_HALT:
    bm->halted = true;
    break;
  case BM_INST_TYPE_DROP:
    if ((error = bm_pop(bm, NULL)) != BM_ERROR_OK)
      return error;
    break;
  case BM_INST_TYPE_JMP_IF_TRUE: {
    BmWord top_value;
    if ((error = bm_pop(bm, &top_value)) != BM_ERROR_OK)
      return error;

    if (top_value != 0)
      bm->pc = inst.operand;
  } break;
  case BM_INST_TYPE_DEBUG_PRINT: {
    BmWord top_value;
    if ((error = bm_pop(bm, &top_value)) != BM_ERROR_OK)
      return error;
    printf("%ld\n", top_value);
  } break;
  case BM_INST_TYPE_DUPLICATE: {
    if (inst.operand >= bm->stack_index)
      return BM_ERROR_STACK_UNDERFLOW;

    BmWord index = bm->stack_index - inst.operand - 1;
    BmWord value = bm->stack[index];

    if ((error = bm_push(bm, value)) != BM_ERROR_OK)
      return error;
  } break;
  case BM_INST_TYPE_NO_OPERATION:
    break;
  default:
    return BM_ERROR_ILLEGAL_INST;
    break;
  }

  return error;
}

BmError bm_step(Bm *bm) {
  BmError error = BM_ERROR_OK;

  BmInst inst;
  if ((error = bm_fetch_inst(bm, &inst)) != BM_ERROR_OK)
    return error;

  if ((error = bm_execute_inst(bm, inst)) != BM_ERROR_OK)
    return error;

  return error;
}

BmError bm_execute_program(Bm *bm, int limit) {
  BmError error = BM_ERROR_OK;
  while (!bm->halted && limit != 0) {
    if ((error = bm_step(bm)) != BM_ERROR_OK)
      return error;

    if (limit > 0)
      limit--;
  }

  return error;
}
