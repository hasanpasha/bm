#include <assert.h>
#include <stdio.h>

#include <nan_box.h>

int main() {
  int x = 0xC0FFEE;
  assert(22.0 / 7.0 == bm_nb_as_double(bm_nb_double(22.0 / 7.0)));
  assert(123456 == bm_nb_as_integer(bm_nb_integer(123456)));
  assert((void *)&x == bm_nb_as_pointer(bm_nb_pointer((void *)&x)));

  printf("OK\n");
  return 0;
}
