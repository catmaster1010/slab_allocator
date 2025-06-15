#include "slab.h"
#include "common.h" // Include your header file here
#include "lib/hashmap.h"
#include "slab_internal.h"

kmem_cache_t _kmem_cache_t_cache;
#define DLL_LIST_ADD(n, prev, next)                                            \
  {                                                                            \
    next->prev = n;                                                            \
    n->next = next;                                                            \
    n->prev = prev;                                                            \
    prev->next = n;                                                            \
  }

kmem_cache_t *
kmem_cache_create(const char *name, /* descriptive name for this cache */
                  size_t size,      /* size of the objects it manages */
                  size_t align,     /* minimum object alignment */
                  void (*constructor)(void *obj),
                  void (*destructor)(void *obj)) {
  kmem_cache_t *cp = (kmem_cache_t *)kmem_calloc(
      sizeof(kmem_cache_t)); // TODO: bootstrap so we no longer rely on malloc
  cp->name = name;
  cp->object_size = size;
  kmem_debug("Object size: %ld\n", size);
  cp->align = align;
  cp->constructor = constructor;
  cp->destructor = destructor;

  // will make sep. cache for hashmaps aswell
  cp->hashmap = (kmem_hashmap_t *)kmem_calloc(
      sizeof(kmem_hashmap_t)); // TODO: bootstrap so we no longer rely on malloc
  kmem_hashmap_create(
      cp->hashmap, KM_NUM_BUCKETS); // in kmem_cache_alloc when implemented...
  return cp;
}

void *kmem_cache_alloc(kmem_cache_t *cp, int kmflag) {
  while (!cp->slabs_partial && !cp->slabs_free) {
    __kmem_cache_grow(cp, kmflag);
  }
  kmem_slab_t *prev;
  kmem_slab_t *next;
  kmem_bufctl_t *buf;
  kmem_slab_t *slab;
  /* try partial slabs first */
  if (cp->slabs_partial) {
    slab = cp->slabs_partial;

    /* If it becomes a full slab, add it to the front*/
    if (slab->ref_count == slab->buf_count) {
      kmem_slab_t *prev = cp->slabs->prev;
      kmem_slab_t *next = cp->slabs->next;
      DLL_LIST_ADD(slab, prev, next);
    }
    /* else, its still a partial slab, we dont need to do anything */
  }
  /* Now try free slabs */
  else if (cp->slabs_free) {
    slab = cp->slabs_free;

    // If this is the last free slab
    if (slab->next == cp->slabs) {
      cp->slabs_free = NULL;
      prev = slab;
      next = slab;
    } else {
      /* set cp->slabs_free to the next slab in line */
      cp->slabs_free = cp->slabs_free->next;
      cp->slabs_free->prev = slab->prev;
      prev = cp->slabs_free->prev;
      next = cp->slabs_free;
    }

    /* add the new slab to partial slabs, remember that there are no partial
     * slabs, thus, we can just set the cp->partial to the slab */
    cp->slabs_partial = slab;
    printf("Prev %p next %p\n", prev, next);
    DLL_LIST_ADD(slab, prev, next);
  }

  buf = slab->buf_front;
  slab->buf_front = slab->buf_front->next;
  slab->ref_count++;
  return buf->buf;
}

// Only support freeing of large caches (For now)
void kmem_cache_free(kmem_cache_t *cp, void *buf) {
  kmem_bufctl_t *bufctl = (kmem_bufctl_t *)kmem_hashmap_get(cp->hashmap, buf);
  __kmem_cache_free_large(cp, bufctl);

  return;
}

int kmem_init() {
  kmem_cache_create("kmem_cache", sizeof(kmem_cache_t), 0, NULL, NULL);

  return 0;
}
