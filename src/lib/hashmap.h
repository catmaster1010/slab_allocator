#ifndef KMEM_HASHMAP_H
#define KMEM_HASHMAP_H
#include "../common.h"
#include <stdbool.h>
#include <stdint.h>

#define KM_NUM_BUCKETS 32
#define KM_KEY_LENGTH sizeof(uintptr_t)
typedef struct kmem_hashmap_entry {
  struct kmem_hashmap_entry *next;
  void *key;
  void *val;
} kmem_hashmap_entry_t;

typedef struct {
  kmem_hashmap_entry_t *entries;
  size_t entry_count;
  size_t items;
} kmem_hashmap_t;

bool kmem_hashmap_create(kmem_hashmap_t *kmem_hashmap, size_t item_count);
bool kmem_hashmap_set(kmem_hashmap_t *kmem_hashmap, void *key, void *val);
void *kmem_hashmap_get(kmem_hashmap_t *kmem_hashmap, void *key);
bool kmem_hashmap_remove(kmem_hashmap_t *kmem_hashmap, void *key);
bool kmem_hashmap_delete(kmem_hashmap_t *kmem_hashmap);
#endif
