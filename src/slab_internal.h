#ifndef SLAB_INTERNAL_H
#define SLAB_INTERNAL_H
#include "slab.h"
void __kmem_cache_grow(kmem_cache_t *cp, int kmflag);
void __kmem_cache_free_large(kmem_cache_t *cp, kmem_bufctl_t *bufctl);
#define DLL_LIST_ADD(n, prev, next)                                            \
  {                                                                            \
    next->prev = n;                                                            \
    n->next = next;                                                            \
    n->prev = prev;                                                            \
    prev->next = n;                                                            \
  }
#endif // !SLAB_INTERNAL_H
