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
  kmem_slab_t *slabs_full;
  kmem_slab_t *slabs_partial;
  kmem_slab_t *slabs_free; /* available for allocation */
  kmem_slab_t *first_free; /*first free slab available for allocation */
  const char *name;
  size_t object_size; /* size of the object*/
  size_t align;
  kmem_slab_t *slabs;
  void (*constructor)(void *, void *priv, int kmflag);
  void (*destructor)(void *, void *priv);
  unsigned long num_active;      /* number of bufs available */
  unsigned long num_allocations; /* number of bufs allocations made */
  kmem_hashmap_t hashmap;
} kmem_cache_t;

kmem_cache_t *
kmem_cache_create(const char *name, /* descriptive name for this cache */
                  size_t size,      /* size of the objects it manages */
                  size_t align,     /* minimum object alignment */
                  void (*constructor)(void *obj, void *priv, int kmflag),
                  void (*destructor)(void *obj, void *priv));
/* returns allocated cache in contstructed state*/
void *kmem_cache_alloc(kmem_cache_t *cp, int kmflag);

int kmem_init();
#endif
