#ifndef BM_H
#define BM_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#if defined(__GNUC__) || defined(__clang__)
#define BM_UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
#define BM_UNREACHABLE() __assume(0)
#else
#include <stdlib.h>
#define BM_UNREACHABLE() abort()
#endif

typedef enum BM_ERROR {
  BM_ERROR_OK,
  BM_ERROR_STACK_OVERFLOW,
  BM_ERROR_STACK_UNDERFLOW,
  BM_ERROR_ILLEGAL_INST,
  BM_ERROR_DIVIDE_BY_ZERO,
  BM_ERROR_ILLEGAL_INST_ACCESS,
  BM_ERROR_ILLEGAL_OPERAND,
  BM_NUM_OF_ERRORS,
} BmError;

const char *bm_error_string(BmError error);

#define BM_STACK_CAP 1024
#define BM_PROGRAM_CAP 1024

typedef uint64_t BmInstAddr;

typedef union BM_WORD {
  uint64_t u64;
  int64_t i64;
  double f64;
  void *ptr;
} BmWord;

static_assert(sizeof(BmWord) == 8, "BmWord is expected to be 8 bytes.");

void bm_word_dump(BmWord word, FILE *stream);

typedef enum BM_INST_TYPE {
  BM_INST_TYPE_NO_OPERATION,
  BM_INST_TYPE_HALT,
  BM_INST_TYPE_PUSH,
  BM_INST_TYPE_PLUS_INT,
  BM_INST_TYPE_MINUS_INT,
  BM_INST_TYPE_MULTIPLY_INT,
  BM_INST_TYPE_DIVIDE_INT,
  BM_INST_TYPE_PLUS_FLOAT,
  BM_INST_TYPE_MINUS_FLOAT,
  BM_INST_TYPE_MULTIPLY_FLOAT,
  BM_INST_TYPE_DIVIDE_FLOAT,
  BM_INST_TYPE_JUMP,
  BM_INST_TYPE_JMP_IF_TRUE,
  BM_INST_TYPE_TEST_EQUALS,
  BM_INST_TYPE_DEBUG_PRINT,
  BM_INST_TYPE_DUPLICATE,
  BM_INST_TYPE_SWAP,
  BM_NUM_OF_INST_TYPES,
} BmInstType;

const char *bm_inst_type_readable_name(BmInstType inst_type);

bool bm_inst_type_has_operand(BmInstType inst_type);

const char *bm_inst_type_string(BmInstType inst_type);

typedef struct BM_INST {
  BmInstType type;
  BmWord operand;
} BmInst;

void bm_inst_dump(BmInst inst, FILE *stream);

typedef struct BM_STACK {
  BmWord ptr[BM_STACK_CAP];
  uint64_t idx;
} BmStack;

void bm_stack_dump(const BmStack *stack, FILE *stream);

BmError bm_stack_push(BmStack *stack, BmWord operand);

BmError bm_stack_pop(BmStack *stack, BmWord *operand_out);

BmError bm_stack_set(BmStack *stack, size_t offset, BmWord operand);

BmError bm_stack_get(BmStack *stack, size_t offset, BmWord *operand_out);

typedef struct BM_PROGRAM {
  BmInst ptr[BM_PROGRAM_CAP];
  uint64_t len;
} BmProgram;

bool bm_program_load_from_file(BmProgram *prg, const char *file_path);

bool bm_program_save_to_file(const BmProgram *prg, const char *file_path);

void bm_program_push(BmProgram *prg, BmInst inst);

typedef struct BM {
  BmStack stack;
  BmProgram program;
  BmInstAddr pc;
  bool halted;
} Bm;

void bm_dump(const Bm *bm, FILE *stream);

BmError bm_fetch_inst(Bm *bm, BmInst *inst_out);

BmError bm_execute_inst(Bm *bm, BmInst inst);

BmError bm_step(Bm *bm);

BmError bm_execute_program(Bm *bm, int limit);

#ifdef BM_IMPLEMENTATION

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
  case BM_NUM_OF_ERRORS:
  default:
    BM_UNREACHABLE();
  }
}

void bm_word_dump(BmWord word, FILE *stream) {
  fprintf(stream, "u64:%lu i64:%ld f64:%lf ptr:%p", word.u64, word.i64,
          word.f64, word.ptr);
}

const char *bm_inst_type_readable_name(BmInstType inst_type) {
  switch (inst_type) {
  case BM_INST_TYPE_NO_OPERATION:
    return "nop";
  case BM_INST_TYPE_HALT:
    return "hlt";
  case BM_INST_TYPE_PUSH:
    return "push";
  case BM_INST_TYPE_PLUS_INT:
    return "plusi";
  case BM_INST_TYPE_MINUS_INT:
    return "minusi";
  case BM_INST_TYPE_MULTIPLY_INT:
    return "multi";
  case BM_INST_TYPE_DIVIDE_INT:
    return "divi";
  case BM_INST_TYPE_PLUS_FLOAT:
    return "plusf";
  case BM_INST_TYPE_MINUS_FLOAT:
    return "minusf";
  case BM_INST_TYPE_MULTIPLY_FLOAT:
    return "multf";
  case BM_INST_TYPE_DIVIDE_FLOAT:
    return "divf";
  case BM_INST_TYPE_JUMP:
    return "jmp";
  case BM_INST_TYPE_JMP_IF_TRUE:
    return "jt";
  case BM_INST_TYPE_TEST_EQUALS:
    return "teq";
  case BM_INST_TYPE_DEBUG_PRINT:
    return "dump";
  case BM_INST_TYPE_DUPLICATE:
    return "dup";
  case BM_INST_TYPE_SWAP:
    return "swap";
  case BM_NUM_OF_INST_TYPES:
  default:
    BM_UNREACHABLE();
  }
}

bool bm_inst_type_has_operand(BmInstType inst_type) {
  switch (inst_type) {
  case BM_INST_TYPE_PUSH:
    return true;
  case BM_INST_TYPE_JUMP:
    return true;
  case BM_INST_TYPE_DUPLICATE:
    return true;
  case BM_INST_TYPE_NO_OPERATION:
  case BM_INST_TYPE_HALT:
  case BM_INST_TYPE_PLUS_INT:
  case BM_INST_TYPE_MINUS_INT:
  case BM_INST_TYPE_MULTIPLY_INT:
  case BM_INST_TYPE_DIVIDE_INT:
  case BM_INST_TYPE_PLUS_FLOAT:
  case BM_INST_TYPE_MINUS_FLOAT:
  case BM_INST_TYPE_MULTIPLY_FLOAT:
  case BM_INST_TYPE_DIVIDE_FLOAT:
  case BM_INST_TYPE_JMP_IF_TRUE:
  case BM_INST_TYPE_TEST_EQUALS:
  case BM_INST_TYPE_DEBUG_PRINT:
  case BM_INST_TYPE_SWAP:
    return false;
  case BM_NUM_OF_INST_TYPES:
  default:
    BM_UNREACHABLE();
  }
}

const char *bm_inst_type_string(BmInstType inst_type) {
  switch (inst_type) {
  case BM_INST_TYPE_NO_OPERATION:
    return "NO_OPERATION";
  case BM_INST_TYPE_HALT:
    return "HALT";
  case BM_INST_TYPE_PUSH:
    return "PUSH";
  case BM_INST_TYPE_PLUS_INT:
    return "PLUS_INT";
  case BM_INST_TYPE_MINUS_INT:
    return "MINUS_INT";
  case BM_INST_TYPE_MULTIPLY_INT:
    return "MULTIPLY_INT";
  case BM_INST_TYPE_DIVIDE_INT:
    return "DIVIDE_INT";
  case BM_INST_TYPE_PLUS_FLOAT:
    return "PLUS_FLOAT";
  case BM_INST_TYPE_MINUS_FLOAT:
    return "MINUS_FLOAT";
  case BM_INST_TYPE_MULTIPLY_FLOAT:
    return "MULTIPLY_FLOAT";
  case BM_INST_TYPE_DIVIDE_FLOAT:
    return "DIVIDE_FLOAT";
  case BM_INST_TYPE_JUMP:
    return "JUMP";
  case BM_INST_TYPE_JMP_IF_TRUE:
    return "JMP_IF_TRUE";
  case BM_INST_TYPE_TEST_EQUALS:
    return "TEST_EQUALS";
  case BM_INST_TYPE_DEBUG_PRINT:
    return "DEBUG_PRINT";
  case BM_INST_TYPE_DUPLICATE:
    return "DUPLICATE";
  case BM_INST_TYPE_SWAP:
    return "SWAP";
  case BM_NUM_OF_INST_TYPES:
  default:
    BM_UNREACHABLE();
  }
}

void bm_inst_dump(BmInst inst, FILE *stream) {
  fprintf(stream, "INST_%s", bm_inst_type_string(inst.type));
  if (bm_inst_type_has_operand(inst.type)) {
    fputc('(', stream);
    bm_word_dump(inst.operand, stream);
    fputc(')', stream);
    // fprintf(stream, "(%ld)", inst.operand.i64);
  }
}

void bm_dump(const Bm *bm, FILE *stream) {
  BmInst inst = bm->program.ptr[bm->pc];
  fprintf(stream, "%04ld:\t", bm->pc);
  bm_inst_dump(inst, stream);
  fprintf(stream, "\t");
  bm_stack_dump(&bm->stack, stream);
  fprintf(stream, "\n");
}

bool bm_program_load_from_file(BmProgram *prg, const char *file_path) {
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
    fprintf(stderr, "Error: program is too big: %lu\n", program_size);
    return false;
  }

  if (fseek(f, 0, SEEK_SET) < 0) {
    fprintf(stderr,
            "Error: could not seek to the beginning of file '%s': %s.\n",
            file_path, strerror(errno));
    return false;
  }

  size_t read_insts = fread(prg->ptr, sizeof(BmInst), program_size, f);
  if (read_insts < program_size) {
    fprintf(stderr,
            "Error: could not load the entire program file '%s' of size %lu.\n",
            file_path, program_size);
    return false;
  }

  if (ferror(f)) {
    fprintf(stderr, "Error: could not read from file '%s': %s.\n", file_path,
            strerror(errno));
    return false;
  }

  prg->len = program_size;

  fclose(f);

  return true;
}

bool bm_program_save_to_file(const BmProgram *prg, const char *file_path) {
  FILE *f = fopen(file_path, "wb");
  if (f == NULL) {
    fprintf(stderr, "Error: could not open file '%s': %s\n", file_path,
            strerror(errno));
    return false;
  }

  fwrite(prg->ptr, sizeof(BmInst), prg->len, f);
  if (ferror(f)) {
    fprintf(stderr, "Error: could not write to file '%s': %s.\n", file_path,
            strerror(errno));
    return false;
  }

  fclose(f);

  return true;
}

void bm_program_push(BmProgram *prg, BmInst inst) {
  assert(prg->len < BM_PROGRAM_CAP);
  prg->ptr[prg->len++] = inst;
}

void bm_stack_dump(const BmStack *stack, FILE *stream) {
  fprintf(stream, "stack:\n");
  for (size_t i = 0; i < stack->idx; i++) {
    BmWord word = stack->ptr[i];
    bm_word_dump(word, stream);
    fputc('\n', stream);
  }
}

BmError bm_stack_push(BmStack *stack, BmWord operand) {
  if (stack->idx >= BM_STACK_CAP)
    return BM_ERROR_STACK_OVERFLOW;

  stack->ptr[stack->idx++] = operand;
  return BM_ERROR_OK;
}

BmError bm_stack_pop(BmStack *stack, BmWord *operand_out) {
  if (stack->idx == 0)
    return BM_ERROR_STACK_UNDERFLOW;

  BmWord operand = stack->ptr[--stack->idx];

  if (operand_out != NULL)
    *operand_out = operand;

  return BM_ERROR_OK;
}

BmError bm_stack_set(BmStack *stack, size_t offset, BmWord operand) {
  const long idx = ((long)stack->idx - 1 - (long)offset);

  if (idx < 0)
    return BM_ERROR_STACK_UNDERFLOW;
  if (idx >= BM_STACK_CAP)
    return BM_ERROR_STACK_OVERFLOW;

  stack->ptr[(size_t)idx] = operand;

  return BM_ERROR_OK;
}

BmError bm_stack_get(BmStack *stack, size_t offset, BmWord *operand_out) {
  const long idx = ((long)stack->idx - 1 - (long)offset);

  if (idx < 0)
    return BM_ERROR_STACK_UNDERFLOW;
  if (idx >= BM_STACK_CAP)
    return BM_ERROR_STACK_OVERFLOW;

  if (operand_out != NULL)
    *operand_out = stack->ptr[(size_t)idx];

  return BM_ERROR_OK;
}

BmError bm_fetch_inst(Bm *bm, BmInst *inst_out) {
  if (bm->pc >= bm->program.len)
    return BM_ERROR_ILLEGAL_INST_ACCESS;

  BmInst inst = bm->program.ptr[bm->pc++];
  if (inst_out != NULL)
    *inst_out = inst;

  return BM_ERROR_OK;
}

BmError bm_execute_inst(Bm *bm, BmInst inst) {
  BmError error = BM_ERROR_OK;

  switch (inst.type) {
  case BM_INST_TYPE_PUSH:
    if ((error = bm_stack_push(&bm->stack, inst.operand)) != BM_ERROR_OK)
      return error;
    break;
  case BM_INST_TYPE_PLUS_INT:
  case BM_INST_TYPE_MINUS_INT:
  case BM_INST_TYPE_MULTIPLY_INT:
  case BM_INST_TYPE_DIVIDE_INT: {
    BmWord ao, bo;
    if ((error = bm_stack_pop(&bm->stack, &bo)) != BM_ERROR_OK)
      return error;
    if ((error = bm_stack_pop(&bm->stack, &ao)) != BM_ERROR_OK)
      return error;

    int64_t a = ao.i64;
    int64_t b = bo.i64;
    int64_t result = 0;

    if (inst.type == BM_INST_TYPE_PLUS_INT) {
      result = a + b;
    } else if (inst.type == BM_INST_TYPE_MINUS_INT) {
      result = a - b;
    } else if (inst.type == BM_INST_TYPE_MULTIPLY_INT) {
      result = a * b;
    } else if (inst.type == BM_INST_TYPE_DIVIDE_INT) {
      if (b == 0)
        return BM_ERROR_DIVIDE_BY_ZERO;
      result = a / b;
    }

    if ((error = bm_stack_push(&bm->stack, (BmWord){.i64 = result})) !=
        BM_ERROR_OK)
      return error;
  } break;
  case BM_INST_TYPE_PLUS_FLOAT:
  case BM_INST_TYPE_MINUS_FLOAT:
  case BM_INST_TYPE_MULTIPLY_FLOAT:
  case BM_INST_TYPE_DIVIDE_FLOAT: {
    BmWord ao, bo;
    if ((error = bm_stack_pop(&bm->stack, &bo)) != BM_ERROR_OK)
      return error;
    if ((error = bm_stack_pop(&bm->stack, &ao)) != BM_ERROR_OK)
      return error;

    double a = ao.f64;
    double b = bo.f64;
    double result = 0;
    if (inst.type == BM_INST_TYPE_PLUS_FLOAT) {
      result = a + b;
    } else if (inst.type == BM_INST_TYPE_MINUS_FLOAT) {
      result = a - b;
    } else if (inst.type == BM_INST_TYPE_MULTIPLY_FLOAT) {
      result = a * b;
    } else if (inst.type == BM_INST_TYPE_DIVIDE_FLOAT) {
      result = a / b;
    }

    if ((error = bm_stack_push(&bm->stack, (BmWord){.f64 = result})) !=
        BM_ERROR_OK)
      return error;
  } break;
  case BM_INST_TYPE_TEST_EQUALS: {
    BmWord ao, bo;
    if ((error = bm_stack_pop(&bm->stack, &bo)) != BM_ERROR_OK)
      return error;
    if ((error = bm_stack_pop(&bm->stack, &ao)) != BM_ERROR_OK)
      return error;

    if ((error = bm_stack_push(&bm->stack, (BmWord){.u64 = ao.u64 == bo.u64}) !=
                 BM_ERROR_OK))
      return error;
  } break;
  case BM_INST_TYPE_JUMP:
    bm->pc = inst.operand.u64;
    break;
  case BM_INST_TYPE_HALT:
    bm->halted = true;
    break;
  case BM_INST_TYPE_JMP_IF_TRUE: {
    BmWord top_value;
    if ((error = bm_stack_pop(&bm->stack, &top_value)) != BM_ERROR_OK)
      return error;

    if (top_value.u64 != 0)
      bm->pc = inst.operand.u64;
  } break;
  case BM_INST_TYPE_DEBUG_PRINT: {
    if (bm->stack.idx <= 0)
      return BM_ERROR_STACK_UNDERFLOW;
    bm_word_dump(bm->stack.ptr[bm->stack.idx - 1], stdout);
    fputc('\n', stdout);
  } break;
  case BM_INST_TYPE_DUPLICATE: {
    if (inst.operand.u64 >= bm->stack.idx)
      return BM_ERROR_STACK_UNDERFLOW;

    BmInstAddr index = bm->stack.idx - inst.operand.u64 - 1;
    BmWord value = bm->stack.ptr[index];

    if ((error = bm_stack_push(&bm->stack, value)) != BM_ERROR_OK)
      return error;
  } break;
  case BM_INST_TYPE_NO_OPERATION:
    break;
  case BM_INST_TYPE_SWAP: {
    BmWord a, b;
    if ((error = bm_stack_get(&bm->stack, 0, &a)) != BM_ERROR_OK)
      return error;
    if ((error = bm_stack_get(&bm->stack, 1, &b)) != BM_ERROR_OK)
      return error;
    if ((error = bm_stack_set(&bm->stack, 0, b)) != BM_ERROR_OK)
      return error;
    if ((error = bm_stack_set(&bm->stack, 1, a)) != BM_ERROR_OK)
      return error;
  } break;
  case BM_NUM_OF_INST_TYPES:
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

#endif // BM_IMPLEMENTATION

#endif // BM_H