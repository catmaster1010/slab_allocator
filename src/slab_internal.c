#include "slab_internal.h"
#include "common.h"
#include "lib/hashmap.h"
#include "slab.h"

void __slab_grow_large(kmem_cache_t *cp) {}
void __slab_grow_small(kmem_cache_t *cp) {}

// ASSUMPTIONS: slab is a newly constructed slab
static inline void __add_slab2cache(kmem_cache_t *cp, kmem_slab_t *slab) {
  cp->num_active += slab->size;
  if (!cp->slabs_free) {
    cp->slabs_free = slab;
  }
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

/* Move a slab to the end, these slabs will then be claimed when
 * `kmem_cache_reap()` is called */
static inline void __remove_slabfromcache(kmem_cache_t *cp, kmem_slab_t *slab) {
  // Already at the end
  if (cp->slabs == slab) {
    return;
  }
  /* else, move it to the end */
  slab->prev->next = slab->next;
  slab->next->prev = slab->prev;

  kmem_slab_t *prev = cp->slabs->prev;
  kmem_slab_t *next = cp->slabs->next;
  DLL_LIST_ADD(slab, prev, next);
}

static inline void __make_bufctl(kmem_cache_t *cp, kmem_slab_t *slab,
                                 kmem_bufctl_t *last_bufctl,
                                 kmem_bufctl_t *bufctl, void **page,
                                 unsigned long i) {

  printf("slab size: %ld, i %ld\n", slab->size, i);
  bufctl = (kmem_bufctl_t *)kmem_calloc(
      sizeof(kmem_bufctl_t)); // allocate a bufctl structure
  assert(bufctl);

  bufctl->slab = slab; // backpointer to the slab
  bufctl->buf =
      (void *)((uintptr_t)*page +
               (uintptr_t)i * cp->object_size); // Calculate where the buffer is
  bufctl->next = NULL;
  last_bufctl->next = bufctl;
  kmem_debug("%p to %p...", bufctl->buf, bufctl);

  kmem_hashmap_set(cp->hashmap, bufctl->buf,
                   bufctl); // bufctl->buf to bufctl hashmap
  kmem_debug("result: %p\n", kmem_hashmap_get(cp->hashmap, bufctl->buf));

  cp->constructor(bufctl); // Construct it
  last_bufctl = bufctl;
  kmem_debug("bufctl has been constructed! \n");
  return;
}

// just support large caches for now... deafult small caches to large ones
void __kmem_cache_grow(kmem_cache_t *cp, int kmflag) {

  // Allocate the actual buffer
  void **page;
  kmem_posix_memalign(page, cp->align,
                      PAGE_SZ); // TODO: Support multiple page objects
  kmem_assert(*page);
  kmem_debug(
      "Page allocation is a success! Location: %p, pointer location: %p\n",
      *page, page);

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
  kmem_debug("A bufctl_t has been made. Adress: %p .\n", last_bufctl);
  last_bufctl->slab = slab;
  last_bufctl->buf = (void *)(*page);
  last_bufctl->next = NULL;

  kmem_hashmap_set(cp->hashmap, last_bufctl->buf, last_bufctl);
  slab->buf_front = last_bufctl;

  // Init the rest of the bufctls
  kmem_bufctl_t *bufctl;
  for (unsigned long i = 1; i < slab->size - 1; i++) {
    bufctl = (kmem_bufctl_t *)kmem_calloc(
        sizeof(kmem_bufctl_t)); // allocate a bufctl structure
    assert(bufctl);

    bufctl->slab = slab; // backpointer to the slab
    bufctl->buf =
        (void *)((uintptr_t)*page +
                 (uintptr_t)i *
                     cp->object_size); // Calculate where the buffer is
    bufctl->next = NULL;
    last_bufctl->next = bufctl;
    kmem_debug("%p to %p...", bufctl->buf, bufctl);

    kmem_hashmap_set(cp->hashmap, bufctl->buf,
                     bufctl); // bufctl->buf to bufctl hashmap
    kmem_debug("result: %p\n", kmem_hashmap_get(cp->hashmap, bufctl->buf));

    cp->constructor(bufctl); // Construct it
    last_bufctl = bufctl;
    kmem_debug("bufctl has been constructed! \n");
  }
  slab->buf_back =
      last_bufctl; /* make sure to set last bufctl of the slab to `buf_back`*/

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
