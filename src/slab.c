#include "slab.h"
#include "common.h"
#include "lib/hashmap.h"
#include "slab_internal.h"

kmem_cache_t *kmem_cache_create(const char *name, size_t size, size_t align,
                                void (*constructor)(void *obj),
                                void (*destructor)(void *obj)) {

  kmem_cache_t *cp = (kmem_cache_t *)kmem_calloc(
      sizeof(kmem_cache_t)); // TODO: bootstrap so we no longer rely on malloc
  cp->name = name;
  cp->object_size = size;
  kmem_debug("Object size: %ld\n", size);

  /* check if it is a power of two */
  if ((align & (align - 1)) == 0) {
    cp->align = align;
  } else {
    kmem_debug("alignment must be a power of two!\n");
    kmem_assert(0);
  }

  cp->constructor = constructor;
  cp->destructor = destructor;

  // TODO: make sep. cache for hashmaps
  cp->hashmap = (kmem_hashmap_t *)kmem_calloc(sizeof(kmem_hashmap_t));
  kmem_hashmap_create(cp->hashmap, KM_NUM_BUCKETS);

  return cp;
}

void kmem_cache_destroy(kmem_cache_t *cp) {
  kmem_assert(cp->num_allocations ==
              0); // ensure that there are no more references to the cache
  __kmem_cache_reap(cp);
  kmem_hashmap_destroy(cp->hashmap);
  kmem_free(cp->hashmap);
  kmem_free(cp);
}

void *kmem_cache_alloc(kmem_cache_t *cp, int kmflag) {
  /* If there are no slabs, that is, both free and partial, we grow the cache*/
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
    if (slab->ref_count + 1 == slab->buf_count) {
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
    DLL_LIST_ADD(slab, prev, next);
  }

  /* remove the buffer from the slab */
  buf = slab->buf_front;
  slab->buf_front = slab->buf_front->next;

  /* increase counts */
  slab->ref_count++;
  cp->num_allocations++;

  return buf->buf;
}

// Only support freeing of large caches (For now)
void kmem_cache_free(kmem_cache_t *cp, void *buf) {
  /* get the respective `bufctl_t` structure from the hashmap */
  kmem_bufctl_t *bufctl = (kmem_bufctl_t *)kmem_hashmap_get(cp->hashmap, buf);
  __kmem_cache_free_large(cp, bufctl);

  cp->num_allocations--;

  return;
}

int kmem_init() {
  kmem_cache_create("kmem_cache", sizeof(kmem_cache_t), 0, NULL, NULL);

  return 0;
}
