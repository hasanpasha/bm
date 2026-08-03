#ifndef STRING_VIEW_H
#define STRING_VIEW_H

#include <stdbool.h>
#include <sys/types.h>

typedef struct STRING_VIEW {
  const char *ptr;
  size_t len;
} StringView;

#define SV "%.*s"
#define SV_ARG(sv) (int)sv.len, sv.ptr

StringView sv_from_cstr(const char *str);

const char *sv_to_cstr(StringView sv, char *buffer, size_t cap);

StringView sv_ltrim(StringView sv);

StringView sv_rtrim(StringView sv);

StringView sv_trim(StringView sv);

StringView sv_chop_by_delim(StringView *sv, char delim);

bool sv_eq(StringView a, StringView b);

bool sv_is_blank(StringView sv);

bool sv_begins_with(StringView sv, StringView slice);

bool sv_ends_with(StringView sv, StringView slice);

bool sv_chop_right(StringView *sv, StringView slice);

signed long int sv_parse_long(StringView sv);

unsigned long int sv_parse_ulong(StringView sv);

double sv_parse_double(StringView sv);

StringView sv_read_file(const char *file_path);

#ifdef STRING_VIEW_IMPLEMENTATION

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "string_view.h"

StringView sv_from_cstr(const char *str) {
  return (StringView){
      .ptr = str,
      .len = (str != NULL) ? strlen(str) : 0,
  };
}

StringView sv_ltrim(StringView sv) {
  size_t i = 0;
  while (i < sv.len && isspace(sv.ptr[i]))
    i++;
  return (StringView){.ptr = sv.ptr + i, .len = sv.len - i};
}

StringView sv_rtrim(StringView sv) {
  size_t i = sv.len;
  while (i > 0 && isspace(sv.ptr[i - 1]))
    i--;
  return (StringView){.ptr = sv.ptr, .len = i};
}

StringView sv_trim(StringView sv) { return sv_rtrim(sv_ltrim(sv)); }

StringView sv_chop_by_delim(StringView *sv, char delim) {
  size_t i = 0;
  while (i < sv->len && sv->ptr[i] != delim)
    i++;

  StringView result = {.ptr = sv->ptr, .len = i};

  sv->ptr += i;
  sv->len -= i;
  if (i < sv->len) {
    sv->ptr += 1;
    sv->len -= 1;
  }

  return result;
}

bool sv_eq(StringView a, StringView b) {
  if (a.len != b.len)
    return false;
  return memcmp(a.ptr, b.ptr, a.len) == 0;
}

bool sv_is_blank(StringView sv) { return sv.len == 0; }

bool sv_begins_with(StringView sv, StringView slice) {
  if (sv.len < slice.len)
    return false;

  for (size_t i = 0; i < slice.len; i++)
    if (sv.ptr[i] != slice.ptr[i])
      return false;

  return true;
}

bool sv_ends_with(StringView sv, StringView slice) {
  if (sv.len < slice.len)
    return false;

  for (size_t i = 0; i < slice.len; i++)
    if (sv.ptr[sv.len - 1 - i] != slice.ptr[slice.len - 1 - i])
      return false;

  return true;
}

bool sv_chop_right(StringView *sv, StringView slice) {
  if (!sv_ends_with(*sv, slice))
    return false;

  sv->len -= slice.len;
  return true;
}

#define BUFFER_CAP 1024
static char buffer[BUFFER_CAP];

const char *sv_to_cstr(StringView sv, char *buffer, size_t cap) {
  assert(sv.len < cap);
  memcpy(buffer, sv.ptr, sv.len);
  buffer[sv.len] = '\0';
  return buffer;
}

signed long int sv_parse_long(StringView sv) {
  char *endptr = NULL;
  const signed long result =
      strtol(sv_to_cstr(sv, buffer, BUFFER_CAP), &endptr, 10);
  assert((size_t)(endptr - buffer) == sv.len);
  return result;
}

unsigned long int sv_parse_ulong(StringView sv) {
  char *endptr = NULL;
  const unsigned long result =
      strtoul(sv_to_cstr(sv, buffer, BUFFER_CAP), &endptr, 10);
  assert((size_t)(endptr - buffer) == sv.len);
  return result;
}

double sv_parse_double(StringView sv) {
  char *endptr = NULL;
  const double result = strtod(sv_to_cstr(sv, buffer, BUFFER_CAP), &endptr);
  assert((size_t)(endptr - buffer) == sv.len);
  return result;
}

StringView sv_read_file(const char *file_path) {
  FILE *f = fopen(file_path, "rb");
  if (f == NULL) {
    fprintf(stderr, "Error: could not open file '%s': %s\n", file_path,
            strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (fseek(f, 0, SEEK_END) < 0) {
    fprintf(stderr, "Error: could not seek to the end of file '%s': %s.\n",
            file_path, strerror(errno));
    exit(EXIT_FAILURE);
  }

  long pos = ftell(f);
  if (pos < 0) {
    fprintf(stderr, "Error: could get size of file '%s': %s.\n", file_path,
            strerror(errno));
    exit(EXIT_FAILURE);
  }

  size_t file_size = (size_t)pos;

  if (fseek(f, 0, SEEK_SET) < 0) {
    fprintf(stderr,
            "Error: could not seek to the beginning of file '%s': %s.\n",
            file_path, strerror(errno));
    exit(EXIT_FAILURE);
  }

  char *buffer = (char *)malloc(file_size);
  if (buffer == NULL) {
    fprintf(stderr,
            "Error: failed to allocate enough memory to read file: '%s'.\n",
            file_path);
    exit(EXIT_FAILURE);
  }

  size_t read_items = fread(buffer, sizeof(char), file_size, f);
  if (read_items < file_size) {
    fprintf(stderr, "Error: could not read the entire file '%s' of size %ld.\n",
            file_path, file_size);
    exit(EXIT_FAILURE);
  }

  if (ferror(f)) {
    fprintf(stderr, "Error: could not read from file '%s': %s.\n", file_path,
            strerror(errno));
    exit(EXIT_FAILURE);
  }

  StringView sv = (StringView){.ptr = buffer, .len = file_size};

  fclose(f);

  return sv;
}

#endif

#endif