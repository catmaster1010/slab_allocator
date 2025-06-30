#ifndef KMEM_HASHMAP_H
#define KMEM_HASHMAP_H
#include "../common.h"
#include <stdbool.h>
#include <stdint.h>

#define KM_NUM_BUCKETS 32

#define KM_KEY_LENGTH sizeof(uintptr_t)

typedef struct kmem_hashmap_entry {
  struct kmem_hashmap_entry *next; // all of the entries are linked.
  void *key;
  void *val;
} kmem_hashmap_entry_t;

typedef struct {
  kmem_hashmap_entry_t *entries; // aka the buckets
  size_t entry_count;
  size_t items;
} kmem_hashmap_t;

/* create a hashmap */
bool kmem_hashmap_create(kmem_hashmap_t *kmem_hashmap, size_t item_count);

/* set `key` to `val` in `kmem_hashmap` */
bool kmem_hashmap_set(kmem_hashmap_t *kmem_hashmap, void *key, void *val);

/* get value of `key` in `kmem_hashmap` */
void *kmem_hashmap_get(kmem_hashmap_t *kmem_hashmap, void *key);

/* remove `key` in `kmem_hashmap` */
bool kmem_hashmap_remove(kmem_hashmap_t *kmem_hashmap, void *key);

/* destroy a hashmap by freeing all related structures*/
bool kmem_hashmap_destroy(kmem_hashmap_t *kmem_hashmap);
#endif
