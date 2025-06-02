#include "slab.h"
#include "common.h" // Include your header file here
#include "lib/hashmap.h"
#include "slab_internal.h"

kmem_cache_t _kmem_cache_t_cache;

kmem_cache_t *
kmem_cache_create(const char *name, /* descriptive name for this cache */
                  size_t size,      /* size of the objects it manages */
                  size_t align,     /* minimum object alignment */
                  void (*constructor)(void *obj, void *priv, int kmflag),
                  void (*destructor)(void *obj, void *priv)) {
  kmem_cache_t *cp = (kmem_cache_t *)malloc(
      sizeof(kmem_cache_t)); // TODO: bootstrap so we no longer rely on malloc
  cp->slabs_full = NULL;     // IN KMEM_cache_alloc wen implemented
  cp->slabs_partial = NULL;  // IN KMEM_cache_alloc wen implemented
  cp->slabs_free = NULL;     // IN KMEM_cache_alloc wen implemented
  cp->name = name;
  cp->object_size = size;
  cp->align = align;
  cp->slabs = NULL; // IN KMEM_cache_alloc wen implemented
  cp->constructor = constructor;
  cp->destructor = destructor;
  cp->num_active = 0;      // IN KMEM_cache_alloc wen implemented
  cp->num_allocations = 0; // IN KMEM_cache_alloc wen implemented

  // will make sep. cache for hashmaps aswell
  kmem_hashmap_create(
      &cp->hashmap, KM_NUM_BUCKETS); // in kmem_cache_alloc when implemented...
  return cp;
}

void *kmem_cache_alloc(kmem_cache_t *cp, int kmflag) {
  while (cp->num_allocations == cp->num_active) {
    __kmem_cache_grow(cp);
    if (kmflag == KM_NOSLEEP)
      break;
    kmem_sleep(1);
  }
  kmem_bufctl_t *buf;
  buf = cp->first_free->buf_front;
  cp->first_free->buf_front = cp->first_free->buf_front->next;
  /*
    if (cp->object_size > KM_SMALL_CACHE) { // its not a small cache
    } else {
      // TODO
    }
    */
  return buf->buf;
}

int kmem_init() {
  kmem_cache_create("kmem_cache", sizeof(kmem_cache_t), 0, NULL, NULL);
  return 0;
}
