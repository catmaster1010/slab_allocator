#include "slab_internal.h"
#include "common.h"
#include "lib/hashmap.h"
#include "slab.h"

// ASSUMPTIONS: slab is a newly constructed slab
static inline void __add_slab2cache(kmem_cache_t *cp, kmem_slab_t *slab) {
  cp->num_active += slab->size;

  /* if there are no free slabs, set cp->slabs_free to that slab */
  if (!cp->slabs_free) {
    cp->slabs_free = slab;
  }

  /* if there are no slabs at all, add it */
  if (cp->slabs) {
    kmem_slab_t *tail = cp->slabs->prev;
    tail->next->prev = slab;
    tail->next = slab;
    slab->prev = tail;
    slab->next = tail->next->prev;
  } else {
    /* else, just add it to the end of the list (where free slabs reside) */
    cp->slabs = slab;
    slab->prev = slab;
    slab->next = slab;
  }
}

/* Move a slab to the end, these slabs will then be claimed when
 * `kmem_cache_reap()` is called
 * ASSUMPTIONS: `ref_count` is 0*/
static inline void __remove_slabfromcache(kmem_cache_t *cp, kmem_slab_t *slab) {

  /* if there are no free slabs, set cp->slabs_free to that slab */
  if (!cp->slabs_free) {
    cp->slabs_free = slab;
  }

  /* if its the only `slabs_partial`` */
  if (cp->slabs_partial->next == cp->slabs_free ||
      cp->slabs_partial->next == cp->slabs ||
      cp->slabs_partial->next == cp->slabs_partial) {
    cp->slabs_partial = NULL;
  }

  // not at the end
  if (!(cp->slabs == slab)) {
    /* move it to the end */
    slab->prev->next = slab->next;
    slab->next->prev = slab->prev;

    kmem_slab_t *prev = cp->slabs->prev;
    kmem_slab_t *next = cp->slabs->next;
    DLL_LIST_ADD(slab, prev, next);
  }
}

// just support large caches for now... deafult small caches to large ones
void __kmem_cache_grow(kmem_cache_t *cp, int kmflag) {

  void *page;
  /* TODO: Support multiple page objects */

  // Allocate the actual buffer
  if (cp->align == 0) { /* if no alignment is specified */
    page = kmem_malloc(PAGE_SZ);
  } else {
    /* if there is an alignment */
    void *page = kmem_aligned_alloc(PAGE_SZ, PAGE_SZ);
  }
  kmem_assert(page);

  // allocate a slab structure
  kmem_slab_t *slab = (kmem_slab_t *)kmem_calloc(sizeof(kmem_slab_t));
  kmem_assert(slab);

  slab->size = PAGE_SZ / cp->object_size; // set # of objects in a slab
  kmem_debug("There are %ld objectes in a slab.\n", slab->size);
  slab->ref_count = 0; // ref_count is 0 until more references are made

  // Init the first bufctl of the chain
  kmem_bufctl_t *last_bufctl =
      (kmem_bufctl_t *)kmem_calloc(sizeof(kmem_bufctl_t));
  assert(last_bufctl);
  last_bufctl->slab = slab;
  last_bufctl->buf = (void *)(page);
  last_bufctl->next = NULL;

  kmem_hashmap_set(cp->hashmap, last_bufctl->buf, last_bufctl);
  slab->buf_front = last_bufctl;
  cp->constructor(last_bufctl->buf); // Construct it

  // Init the rest of the bufctls
  kmem_bufctl_t *bufctl;
  for (unsigned long i = 1; i < slab->size - 1; i++) {
    bufctl = (kmem_bufctl_t *)kmem_calloc(
        sizeof(kmem_bufctl_t)); // allocate a bufctl structure
    assert(bufctl);

    bufctl->slab = slab; // backpointer to the slab
    bufctl->buf =
        (void *)((uintptr_t)page +
                 (uintptr_t)i *
                     cp->object_size); // Calculate where the buffer is
    last_bufctl->next = bufctl;

    kmem_hashmap_set(cp->hashmap, bufctl->buf,
                     bufctl); // bufctl->buf to bufctl hashmap

    cp->constructor(bufctl->buf); // Construct it
    last_bufctl = bufctl;
  }
  slab->buf_back =
      last_bufctl; /* make sure to set last bufctl of the slab to `buf_back`*/
  last_bufctl->next = NULL;

  /* add the finished and constructed slab to the cache */
  __add_slab2cache(cp, slab);
  return;
}

// Assumptions: returned structure is in its constructed state
void __kmem_cache_free_large(kmem_cache_t *cp, kmem_bufctl_t *bufctl) {
  kmem_slab_t *slab = bufctl->slab;
  // add buf to slab
  slab->buf_back->next = bufctl;
  slab->ref_count--;

  // TODO: Keep a working set for 10 seconds
  if (slab->ref_count == 0) {
    __remove_slabfromcache(cp, slab);
  }
}
