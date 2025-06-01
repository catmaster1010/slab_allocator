#ifndef SLAB_H
#define SLAB_H
#include <stddef.h>

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
  unsigned int buf_count;
  unsigned int ref_count;
} kmem_slab_t;

typedef struct kmem_cache {
  kmem_slab_t *slabs_full;
  kmem_slab_t *slabs_partial;
  kmem_slab_t *slabs_free;
  char *name;
  size_t object_size; /* size of the object*/
  size_t align;
  void (*constructor)(void *, void *priv, int kmflag);
  void (*deconstructor)(void *, void *priv);
} kmem_cache_t;

kmem_cache_t *
kmem_cache_create(char *name,   /* descriptive name for this cache */
                  size_t size,  /* size of the objects it manages */
                  size_t align, /* minimum object alignment */
                  int (*constructor)(void *obj, void *priv, int kmflag),
                  void (*destructor)(void *obj, void *priv));

void kmem_init(int (*printf)(const char *format, ...),
               int (*posix_memalign)(void **memptr, size_t alignment,
                                     size_t size));
#endif
