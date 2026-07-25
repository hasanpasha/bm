#ifndef BM_H
#define BM_H

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

typedef uint64_t BmWord;

typedef enum BM_INST_TYPE {
  BM_INST_TYPE_NOP,
  BM_INST_TYPE_HLT,
  BM_INST_TYPE_PUSH,
  BM_INST_TYPE_PLUS,
  BM_INST_TYPE_MINUS,
  BM_INST_TYPE_MULT,
  BM_INST_TYPE_DIV,
  BM_INST_TYPE_JMP,
  BM_INST_TYPE_DROP,
  BM_INST_TYPE_JNZ,
  BM_INST_TYPE_TEQ,
  BM_INST_TYPE_DUMP,
  BM_INST_TYPE_DUP,
} BmInstType;

const char *bm_inst_type_string(BmInstType inst_type);

typedef struct BM_INST {
  BmInstType type;
  BmWord operand;
} BmInst;

void bm_inst_dump(BmInst inst, FILE *stream);

typedef struct BM {
  BmWord stack[BM_STACK_CAP];
  BmWord stack_index;

  BmInst program[BM_PROGRAM_CAP];
  BmWord program_size;
  BmWord pc;

  bool halted;
} Bm;

void bm_dump(const Bm *bm, FILE *stream);

void bm_stack_dump(const Bm *bm, FILE *stream);

void bm_load_program_from_memory(Bm *bm, BmInst *program, BmWord program_size);

bool bm_load_program_from_file(Bm *bm, const char *file_path);

bool bm_save_program_to_file(const Bm *bm, const char *file_path);

BmError bm_push(Bm *bm, BmWord operand);

BmError bm_pop(Bm *bm, BmWord *operand_out);

BmError bm_fetch_inst(Bm *bm, BmInst *inst_out);

BmError bm_execute(Bm *bm, BmInst inst);

BmError bm_step(Bm *bm);

#endif // BM_H