#ifndef ARENA_H
#define ARENA_H

#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct ARENA {
  unsigned char *data;
  size_t offset;
  size_t capacity;
} Arena;

bool arena_init(Arena *arena, size_t capacity);
void arena_deinit(Arena *arena);

void *arena_alloc(Arena *arena, size_t size);

void arena_reset(Arena *arena);

size_t arena_save(Arena *arena);
void arena_restore(Arena *arena, size_t offset);

#define arena_new(arena, type) ((type *)arena_alloc((arena), sizeof(type)))

#define arena_new_array(arena, type, count)                                    \
  ((type *)arena_alloc((arena), sizeof(type) * (count)))

bool arena_init(Arena *arena, size_t capacity) {
  void *data = malloc(capacity);
  if (data == NULL)
    return false;

  arena->data = data;
  arena->offset = 0;
  arena->capacity = capacity;
  return true;
}

void arena_deinit(Arena *arena) {
  assert(arena != NULL);

  free(arena->data);
  arena->data = NULL;
  arena->offset = 0;
  arena->capacity = 0;
}

void *arena_alloc(Arena *arena, size_t size) {
  assert(arena != NULL);

  const size_t alignment = alignof(max_align_t);

  size_t remainder = arena->offset % alignment;
  size_t padding = remainder ? alignment - remainder : 0;

  if (padding > arena->capacity - arena->offset)
    return NULL;

  if (size > arena->capacity - arena->offset - padding)
    return NULL;

  arena->offset += padding;

  void *ptr = arena->data + arena->offset;
  arena->offset += size;

  return ptr;
}

void arena_reset(Arena *arena) {
  assert(arena != NULL);

  arena->offset = 0;
}

size_t arena_save(Arena *arena) {
  assert(arena != NULL);

  return arena->offset;
}

void arena_restore(Arena *arena, size_t offset) {
  assert(arena != NULL);
  assert(offset <= arena->capacity);

  arena->offset = offset;
}

#endif