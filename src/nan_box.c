#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#include <nan_box.h>

#define TYPE_BITS 3ULL
#define VALUE_BITS 48ULL

#define TYPE_MASK (((1ULL << TYPE_BITS) - 1ULL) << VALUE_BITS)
#define VALUE_MASK ((1ULL << VALUE_BITS) - 1ULL)

static uint64_t nb_as_ulong(BmNanBox nb) {
  uint64_t value;
  memcpy(&value, &nb, sizeof(value));
  return value;
}

static BmNanBox ulong_as_nb(uint64_t val) {
  BmNanBox nb;
  memcpy(&nb, &val, sizeof(nb));
  return nb;
}

inline static BmNanBoxType get_type(BmNanBox nb) {
  return (nb_as_ulong(nb) & TYPE_MASK) >> VALUE_BITS;
}

inline static BmNanBox set_type(BmNanBox nb, BmNanBoxType type) {
  return ulong_as_nb(
      (nb_as_ulong(nb) & ~TYPE_MASK) |
      (((uint64_t)type & (TYPE_MASK >> VALUE_BITS)) << VALUE_BITS));
}

inline static uint64_t get_value(BmNanBox nb) {
  return nb_as_ulong(nb) & VALUE_MASK;
}

inline static BmNanBox set_value(BmNanBox nb, uint64_t val) {
  return ulong_as_nb((nb_as_ulong(nb) & ~VALUE_MASK) | (val & VALUE_MASK));
}

void bm_nb_type_dump(BmNanBoxType type, FILE *stream) {
  switch (type) {
  case BM_NAN_BOX_TYPE_DOUBLE:
    fprintf(stream, "DOUBLE");
    break;
  case BM_NAN_BOX_TYPE_INTEGER:
    fprintf(stream, "INTEGER");
    break;
  case BM_NAN_BOX_TYPE_POINTER:
    fprintf(stream, "POINTER");
    break;
  default:
    break;
  }
}

BmNanBoxType bm_nb_get_type(BmNanBox nb) {
  if (!isnan(nb)) {
    return BM_NAN_BOX_TYPE_DOUBLE;
  } else {
    return get_type(nb);
  }
}

void bm_nb_dump(BmNanBox nb, FILE *stream) {
  BmNanBoxType type = bm_nb_get_type(nb);
  bm_nb_type_dump(type, stream);
  fputc('(', stream);
  switch (type) {
  case BM_NAN_BOX_TYPE_DOUBLE:
    fprintf(stream, "%lf", bm_nb_as_double(nb));
    break;
  case BM_NAN_BOX_TYPE_INTEGER:
    fprintf(stream, "%ld", bm_nb_as_integer(nb));
    break;
  case BM_NAN_BOX_TYPE_POINTER:
    fprintf(stream, "%p", bm_nb_as_pointer(nb));
    break;
  }
  fputc(')', stream);
}

BmNanBox bm_nb_double(double value) { return value; }

BmNanBox bm_nb_integer(uint64_t value) {
  return set_value(set_type(NAN, BM_NAN_BOX_TYPE_INTEGER), value);
}

BmNanBox bm_nb_pointer(void *ptr) {
  return set_value(set_type(NAN, BM_NAN_BOX_TYPE_POINTER), (uint64_t)ptr);
}

double bm_nb_as_double(BmNanBox nb) {
  assert(bm_nb_get_type(nb) == BM_NAN_BOX_TYPE_DOUBLE);
  return nb;
}

uint64_t bm_nb_as_integer(BmNanBox nb) {
  assert(bm_nb_get_type(nb) == BM_NAN_BOX_TYPE_INTEGER);
  return get_value(nb);
}

void *bm_nb_as_pointer(BmNanBox nb) {
  assert(bm_nb_get_type(nb) == BM_NAN_BOX_TYPE_POINTER);
  return (void *)get_value(nb);
}
