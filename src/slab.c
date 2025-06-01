#include "slab.h"

kmem_cache_t *kmem_cache_create(char *name, size_t size, size_t align,
                                int (*constructor)(void *obj, void *priv,
                                                   int kmflag),
                                void (*destructor)(void *obj, void *priv)) {
  return 0;
}
