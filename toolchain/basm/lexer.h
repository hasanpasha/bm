#ifndef BASM_LEXER_H
#define BASM_LEXER_H

#define STRING_VIEW_IMPLEMENTATION
#include <string_view.h>

typedef struct BASM_LEXER {
  StringView source;
  size_t start, current;
} BasmLexer;

static BasmLexer basm_lexer_init(StringView source) {
  return (BasmLexer){.source = source, .current = 0, .start = 0};
}

static bool basm_lexer_is_at_end(const BasmLexer *lexer) {
  return lexer->current >= lexer->source.len;
}

static char basm_lexer_current_char(const BasmLexer *lexer) {
  if (basm_lexer_is_at_end(lexer))
    return 0;
  return lexer->source.ptr[lexer->current];
}

static char basm_lexer_advance(BasmLexer *lexer) {
  char cur = basm_lexer_current_char(lexer);
  if (!basm_lexer_is_at_end(lexer))
    lexer->current++;
  return cur;
}

static StringView basm_lexer_current_slice(BasmLexer *lexer) {
  return (StringView){
      .ptr = lexer->source.ptr + lexer->start,
      .len = lexer->current - lexer->start,
  };
}

static void basm_lexer_skip_whitespaces(BasmLexer *lexer) {
  while (!basm_lexer_is_at_end(lexer)) {
    // skip all whitespaces except new line
    switch (basm_lexer_current_char(lexer)) {
    case ' ':
    case '\t':
    case '\v':
    case '\f':
    case '\r':
      basm_lexer_advance(lexer);
      break;
    default:
      lexer->start = lexer->current;
      return;
    }
  }
}

static void basm_lexer_skip_comment(BasmLexer *lexer) {
  if (!basm_lexer_is_at_end(lexer) && basm_lexer_current_char(lexer) == '#') {
    while (!basm_lexer_is_at_end(lexer) &&
           basm_lexer_current_char(lexer) != '\n')
      (void)basm_lexer_advance(lexer);
  }
  lexer->start = lexer->current;
}

static bool basm_lexer_is_num(char c) { return isdigit(c) || c == '.'; }

static bool basm_lexer_is_ident_start(char c) { return isalpha(c) || c == '_'; }

static bool basm_lexer_is_ident(char c) {
  return basm_lexer_is_ident_start(c) || isalnum(c);
}

static BasmToken basm_lexer_token(BasmLexer *lexer, BasmTokenKind kind) {
  return (BasmToken){.kind = kind, .lexeme = basm_lexer_current_slice(lexer)};
}

static BasmToken basm_lexer_advance_with_token(BasmLexer *lexer,
                                               BasmTokenKind kind) {
  (void)basm_lexer_advance(lexer);
  return basm_lexer_token(lexer, kind);
}

static BasmToken basm_lexer_next_token(BasmLexer *lexer) {
  basm_lexer_skip_whitespaces(lexer);
  basm_lexer_skip_comment(lexer);

  if (basm_lexer_is_at_end(lexer)) {
    return (BasmToken){.kind = BASM_TOKEN_KIND_END_OF_INPUT};
  }

  switch (basm_lexer_current_char(lexer)) {
  case ':':
    return basm_lexer_advance_with_token(lexer, BASM_TOKEN_KIND_COLON);
  case '-':
    return basm_lexer_advance_with_token(lexer, BASM_TOKEN_KIND_MINUS);
  case '%':
    return basm_lexer_advance_with_token(lexer, BASM_TOKEN_KIND_PERCENT);
  case '"': {
    (void)basm_lexer_advance(lexer);
    lexer->start++;
    while (!basm_lexer_is_at_end(lexer) &&
           basm_lexer_current_char(lexer) != '"') {
      (void)basm_lexer_advance(lexer);
    }
    const BasmToken tok = basm_lexer_token(lexer, BASM_TOKEN_KIND_STRING);
    (void)basm_lexer_advance(lexer);
    return tok;
  } break;
  case '\n':
    return basm_lexer_advance_with_token(lexer, BASM_TOKEN_KIND_NEW_LINE);
  default:
    if (basm_lexer_is_ident_start(basm_lexer_current_char(lexer))) {
      while (!basm_lexer_is_at_end(lexer) &&
             basm_lexer_is_ident(basm_lexer_current_char(lexer))) {
        basm_lexer_advance(lexer);
      }
      return basm_lexer_token(lexer, BASM_TOKEN_KIND_IDENTIFIER);
    } else if (basm_lexer_is_num(basm_lexer_current_char(lexer))) {
      BasmTokenKind num_kind = BASM_TOKEN_KIND_INTEGER;
      while (!basm_lexer_is_at_end(lexer) &&
             basm_lexer_is_num(basm_lexer_current_char(lexer))) {
        if (basm_lexer_current_char(lexer) == '.') {
          if (num_kind != BASM_TOKEN_KIND_FLOAT) {
            num_kind = BASM_TOKEN_KIND_FLOAT;
          } else {
            fprintf(stderr, "Error: float can't contain more than one '.'\n");
            exit(1);
          }
        }
        (void)basm_lexer_advance(lexer);
      }
      return basm_lexer_token(lexer, num_kind);
    } else {
      fprintf(stderr, "Error: unknown character '%c' at %lu.\n",
              basm_lexer_current_char(lexer), lexer->current);
      exit(1);
    }

    break;
  }
}

#endif