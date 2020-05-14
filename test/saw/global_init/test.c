#include <stdint.h>

uint32_t x = 10;

uint32_t *y;

void f(uint32_t *r, uint32_t *a, uint32_t *b) {
  r[0] = a[0] + b[0];
}

void g(uint32_t *r, uint32_t* a) {
  f(r, a, y);
}
