#ifndef STRING_H
#define STRING_H
#include <stddef.h>
int strlen(const char *f);
int memcmp(const void *s1, const void *s2, size_t size);
void memcpy(void *dest, const void *src, size_t n);
#endif
