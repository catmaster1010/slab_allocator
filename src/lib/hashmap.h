#ifndef KMEM_HASHMAP_H
#define KMEM_HASHMAP_H
#include "../common.h"
#include <stdbool.h>
#include <stdint.h>

#define KM_NUM_BUCKETS 32

typedef struct kmem_hashmap_entry {
  struct kmem_hashmap_entry *next;
  uint64_t key_length;
  char *key;
  void *val;
} kmem_hashmap_entry_t;

typedef struct {
  kmem_hashmap_entry_t *entries;
  uint64_t entry_count;
  uint64_t items;
} kmem_hashmap_t;

bool kmem_hashmap_create(kmem_hashmap_t *kmem_hashmap, size_t item_count);
bool kmem_hashmap_set(kmem_hashmap_t *kmem_hashmap, char *key, void *val);
void *kmem_hashmap_get(kmem_hashmap_t *kmem_hashmap, char *key);
bool kmem_hashmap_remove(kmem_hashmap_t *kmem_hashmap, char *key);
bool kmem_hashmap_delete(kmem_hashmap_t *kmem_hashmap);
#endif
