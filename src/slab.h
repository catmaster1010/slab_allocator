#ifndef SLAB_H
#define SLAB_H
#include "common.h"
#include "lib/hashmap.h"
#include <stddef.h>

#define KM_SLEEP 0x00
#define KM_NOSLEEP 0x01
#define KM_SMALL_CACHE PAGE_SZ / 8
struct kmem_slab;

/* Singly linked */
typedef struct kmem_bufctl {
  struct kmem_slab *slab;
  struct kmem_bufctl *next;
  void *buf;
} kmem_bufctl_t;

/* Doubly linked */
typedef struct kmem_slab {
  struct kmem_slab *next;
  struct kmem_slab *prev;
  kmem_bufctl_t *buf_front;
  kmem_bufctl_t *buf_back;
  unsigned int buf_count; /*number of bufers in this slab*/
  unsigned int ref_count; /* number of refrences to this slab*/
  size_t size;
} kmem_slab_t;

typedef struct kmem_cache {
  kmem_slab_t *slabs_partial; /* pointer to the first partial slab */
  kmem_slab_t *slabs_free;    /* available for allocation */
  const char *name;           /* descriptive name for the cache*/
  size_t object_size;         /* size of the object*/
  size_t align;               /* specified alignment */
  kmem_slab_t *slabs; /*pointer to first slab, basically pointer to full slabs
                         (not in all cases though) */
  void (*constructor)(void *);   /* constructor of the object*/
  void (*destructor)(void *);    /*destructor of the object */
  unsigned long num_active;      /* number of bufs available */
  unsigned long num_allocations; /* number of bufs allocations made */
  kmem_hashmap_t *hashmap;
} kmem_cache_t;

/* returns allocated cache in contstructed state*/
kmem_cache_t *kmem_cache_create(
    const char *name,               /* descriptive name for this cache */
    size_t size,                    /* size of the objects it manages */
    size_t align,                   /* minimum object alignment */
    void (*constructor)(void *obj), /*constructor function for the object*/
    void (*destructor)(void *obj)); /*destructor function for the object*/

/* allocate an object from the cache*/
void *kmem_cache_alloc(kmem_cache_t *cp, int kmflag);

/* free an object to the cache, does not automatically release back into the
 * memmory arena, rather if the buffer returned to the slab becomes completely
 * free, the slab is put onto the back of the slab list of the respective
 * cache to be reclaimed by `kmem_cache_reap` when called. `kmem_cache_free`
 * retains a 10 second working set to prevent thrashing*/
void kmem_cache_free(kmem_cache_t *cp, void *buf);

/* Deletes and frees all supporting structures for a  `kmem_cache_t` pointed to
 * by cp*/
void kmem_cache_destroy(kmem_cache_t *cp);
int kmem_init();
#endif
