#include "slab_internal.h"
#include "slab.h"

void __slab_grow_large(kmem_cache_t *cp) {}
void __slab_grow_small(kmem_cache_t *cp) {}
void __kmem_cache_grow(kmem_cache_t *cp) {
  cp->object_size > KM_SMALL_CACHE ? __slab_grow_large(cp)
                                   : __slab_grow_small(cp);
}
