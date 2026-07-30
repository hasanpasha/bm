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
  BM_INST_TYPE_PLUS,
  BM_INST_TYPE_MINUS,
  BM_INST_TYPE_MULTIPLY,
  BM_INST_TYPE_DIVIDE,
  BM_INST_TYPE_JUMP,
  BM_INST_TYPE_DROP,
  BM_INST_TYPE_JMP_IF_TRUE,
  BM_INST_TYPE_TEST_EQUALS,
  BM_INST_TYPE_DEBUG_PRINT,
  BM_INST_TYPE_DUPLICATE,
} BmInstType;

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

#endif // BM_H