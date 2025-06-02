#include "slab_internal.h"
#include "common.h"
#include "lib/hashmap.h"
#include "slab.h"
#include <stdlib.h>

void __slab_grow_large(kmem_cache_t *cp) {}
void __slab_grow_small(kmem_cache_t *cp) {}

// ASSUMPTIONS: there are no free slabs
static inline void __add_slab2cache(kmem_cache_t *cp, kmem_slab_t *slab) {
  cp->num_active += slab->size;
  cp->first_free = slab;
  if (cp->slabs) {
    kmem_slab_t *tail = cp->slabs->prev;
    tail->next->prev = slab;
    tail->next = slab;
    slab->prev = tail;
    slab->next = tail->next->prev;
  } else {
    cp->slabs = slab;
    slab->prev = slab;
    slab->next = slab;
  }
}

// just support large caches for now... deafult small caches to large ones
void __kmem_cache_grow(kmem_cache_t *cp, int kmflag) {

  void **page;
  kmem_posix_memalign(page, cp->align,
                      PAGE_SZ); // TODO: Support multiple page objects
  kmem_assert(*page);
  kmem_debug(
      "Page allocation is a success! Location: %p, pointer location: %p\n",
      *page, page);

  kmem_slab_t *slab = (kmem_slab_t *)kmem_calloc(sizeof(kmem_slab_t));
  kmem_assert(slab);
  slab->size = PAGE_SZ / cp->object_size;
  kmem_debug("There are %ld objectes in a slab.\n", slab->size);
  slab->ref_count = 0;

  kmem_bufctl_t *last_bufctl =
      (kmem_bufctl_t *)kmem_calloc(sizeof(kmem_bufctl_t));
  assert(last_bufctl);
  kmem_debug("A bufctl_t has been made. Adress: %p .\n", last_bufctl);
  last_bufctl->slab = slab;
  last_bufctl->buf = (void *)(*page);
  last_bufctl->next = NULL;

  kmem_hashmap_set(&cp->hashmap, last_bufctl->buf, last_bufctl);
  slab->buf_front = last_bufctl;

  kmem_bufctl_t *bufctl;
  for (unsigned long i = 1; i < slab->size - 1; i++) {
    bufctl = (kmem_bufctl_t *)kmem_calloc(sizeof(kmem_bufctl_t));
    assert(bufctl);
    bufctl->slab = slab;
    bufctl->buf = (void *)((uintptr_t)*page + (uintptr_t)i * cp->object_size);
    bufctl->next = NULL;
    last_bufctl->next = bufctl;
    printf("%p to %p...", bufctl->buf, bufctl);

    kmem_hashmap_set(&cp->hashmap, bufctl->buf, bufctl);
    printf("result: %p\n", kmem_hashmap_get(&cp->hashmap, bufctl->buf));
    last_bufctl = bufctl;
  }
  __add_slab2cache(cp, slab);
  return;
}
