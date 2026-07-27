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

StringView sv_slice(StringView sv, size_t begin, ssize_t end) {

  size_t len = 0;
  if (end < 0) {
    if (labs(end) <= sv.len) {
      len = sv.len + end;
    }
  } else if (end > 0) {
    len = (size_t)end;
  }

  if (begin > len) {
    len = 0;
  } else {
    len -= begin;
  }

  return (StringView){
      .ptr = sv.ptr + begin,
      .len = len,
  };
}

StringView sv_ltrim(StringView sv) {
  size_t i = 0;
  while (i < sv.len && isspace(sv.ptr[i]))
    i++;
  return (StringView){.ptr = sv.ptr + i, .len = sv.len - i};
}

StringView sv_rtrim(StringView sv) {
  size_t i = sv.len - 1;
  while (i > 0 && isspace(sv.ptr[i]))
    i--;
  return (StringView){.ptr = sv.ptr, .len = i + 1};
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

unsigned long sv_parse_ulong(StringView sv) {
  unsigned long result = 0;
  for (size_t i = 0; i < sv.len && isdigit(sv.ptr[i]); i++) {
    result = (result * 10) + (size_t)(sv.ptr[i] - '0');
  }

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