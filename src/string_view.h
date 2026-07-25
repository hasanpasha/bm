#ifndef STRING_VIEW_H
#define STRING_VIEW_H

#include <stdbool.h>

typedef struct STRING_VIEW {
  const char *ptr;
  size_t len;
} StringView;

#define SV "%.*s"
#define SV_ARG(sv) (int)sv.len, sv.ptr

StringView sv_from_cstr(const char *str);

StringView sv_ltrim(StringView sv);

StringView sv_rtrim(StringView sv);

StringView sv_trim(StringView sv);

StringView sv_chop_by_delim(StringView *sv, char delim);

bool sv_eq(StringView a, StringView b);

bool sv_is_blank(StringView sv);

bool sv_begins_with(StringView sv, StringView slice);

unsigned long sv_parse_ulong(StringView sv);

StringView sv_read_file(const char *file_path);

#endif