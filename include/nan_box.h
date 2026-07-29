#ifndef BM_NAN_BOX
#define BM_NAN_BOX

#include <stdint.h>

// must never be bigger than 8
typedef enum BM_NAN_BOX_TYPE {
  BM_NAN_BOX_TYPE_DOUBLE = 0,
  BM_NAN_BOX_TYPE_INTEGER = 1,
  BM_NAN_BOX_TYPE_POINTER = 2,
} BmNanBoxType;

typedef double BmNanBox;

BmNanBoxType bm_nb_get_type(BmNanBox nb);

BmNanBox bm_nb_double(double value);
BmNanBox bm_nb_integer(uint64_t value);
BmNanBox bm_nb_pointer(void *ptr);

double bm_nb_as_double(BmNanBox nb);
uint64_t bm_nb_as_integer(BmNanBox nb);
void *bm_nb_as_pointer(BmNanBox nb);

#endif