#ifndef BASM_TOKEN_H
#define BASM_TOKEN_H

#include <stdio.h>
#include <stdlib.h>

#include <string_view.h>

typedef enum BASM_TOKEN_KIND {
  BASM_TOKEN_KIND_IDENTIFIER,
  BASM_TOKEN_KIND_STRING,
  BASM_TOKEN_KIND_INTEGER,
  BASM_TOKEN_KIND_FLOAT,
  BASM_TOKEN_KIND_COLON,
  BASM_TOKEN_KIND_MINUS,
  BASM_TOKEN_KIND_PERCENT,
  BASM_TOKEN_KIND_NEW_LINE,
  BASM_TOKEN_KIND_END_OF_INPUT,
} BasmTokenKind;

static const char *basm_token_kind_string(BasmTokenKind kind) {
  switch (kind) {
  case BASM_TOKEN_KIND_IDENTIFIER:
    return "IDENTIFIER";
  case BASM_TOKEN_KIND_STRING:
    return "STRING";
  case BASM_TOKEN_KIND_INTEGER:
    return "INTEGER";
  case BASM_TOKEN_KIND_FLOAT:
    return "FLOAT";
  case BASM_TOKEN_KIND_COLON:
    return "COLON";
  case BASM_TOKEN_KIND_MINUS:
    return "MINUS";
  case BASM_TOKEN_KIND_PERCENT:
    return "PERCENT";
  case BASM_TOKEN_KIND_NEW_LINE:
    return "NEW_LINE";
  case BASM_TOKEN_KIND_END_OF_INPUT:
    return "END_OF_INPUT";
  default:
    fprintf(stderr, "Error: unknown token kind: %d\n", kind);
    exit(1);
  }
}

typedef struct BASM_TOKEN {
  BasmTokenKind kind;
  StringView lexeme;
} BasmToken;

#endif