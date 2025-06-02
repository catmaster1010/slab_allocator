#include "string.h"
#include <stdint.h>
int strlen(const char *f) {
  int ct = 0;
  for (; f[ct] != '\0'; ct++)
    ;
  return ct;
}

// returns 1 or -1 if not equal
int memcmp(const void *s1, const void *s2, size_t size) {
  const uint8_t *p1 = (const uint8_t *)s1;
  const uint8_t *p2 = (const uint8_t *)s2;

  for (size_t i = 0; i < size; i++)
    if (p1[i] != p2[i]) {
      return p1[i] < p2[i] ? -1 : 1; // different (with comparison)
    }

  return 0; // equal
}
void memcpy(void *dest, const void *src, size_t n) {
  for (uint64_t i = 0; i < n; i++) {
    ((uint8_t *)dest)[i] = ((const uint8_t *)src)[i];
  }
}
