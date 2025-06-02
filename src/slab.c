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
  kmem_cache_t *cp = (kmem_cache_t *)kmem_calloc(
      sizeof(kmem_cache_t)); // TODO: bootstrap so we no longer rely on malloc
  cp->name = name;
  cp->object_size = size;
  kmem_debug("Object size: %ld\n", size);
  cp->align = align;
  cp->constructor = constructor;
  cp->destructor = destructor;

  // will make sep. cache for hashmaps aswell
  kmem_hashmap_create(
      &cp->hashmap, KM_NUM_BUCKETS); // in kmem_cache_alloc when implemented...
  return cp;
}

void *kmem_cache_alloc(kmem_cache_t *cp, int kmflag) {
  while (!cp->slabs) {
    __kmem_cache_grow(cp, kmflag);
  }
  kmem_bufctl_t *buf;
  buf = cp->first_free->buf_front;
  cp->first_free->buf_front = cp->first_free->buf_front->next;
  return buf->buf;
}

int kmem_init() {
  kmem_cache_create("kmem_cache", sizeof(kmem_cache_t), 0, NULL, NULL);
  return 0;
}
