#include <stdint.h>
#include <stdio.h>

#include <gmp.h>

uint64_t q_array[4] = {0x1               , 0xFFFFFFFFFFFFFFFF,
                       0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFF43};

mpz_t glo;

void mpz_init (mpz_t x) {
  x->_mp_alloc = 256;
  x->_mp_size = 4;
  x->_mp_d = malloc(4 * sizeof(uint64_t));
  uint32_t i;
  for(i = 0; i < 4; i++) {
    x->_mp_d[i] = 0;
  }
}

void glo_init () {
  mpz_init(glo);
}

void mpz_import (mpz_t rop, size_t count, int order, size_t size, int endian, size_t nails, const void *op) {
  uint32_t i;
  for(i = 0; i < count; i++) {
    uint64_t x =  ((uint64_t *)op)[i];
    printf("%u\n", i);
    rop->_mp_d[i] = x;
    printf("%u\n", rop->_mp_d[i]);
  }  
}

void glo_import () {
  mpz_init(glo);
  mpz_import(glo, 4, 1, 8, 0, 0, q_array);
};

void mpz_set_ui (mpz_t rop, unsigned long int op) {}
