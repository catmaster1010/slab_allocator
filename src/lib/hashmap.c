#include "hashmap.h"
#include "../slab.h"
#include "string.h"
#include <stdint.h>

#define INDEX(KEY, ENTRY_COUNT) {(uintptr_t)KEY % ENTRY_COUNT}

static inline kmem_hashmap_entry_t *
kmem_hashmap_entry_get(kmem_hashmap_t *kmem_hashmap, void *key) {
  size_t index = INDEX((size_t)key, (size_t)kmem_hashmap->entry_count);
  kmem_hashmap_entry_t *entry = &kmem_hashmap->entries[index];
  while (entry) {
    if (entry->key == NULL)
      return entry;
    if (entry->key == key) {
      break;
    }
    entry = entry->next;
  }
  return entry;
}

bool kmem_hashmap_create(kmem_hashmap_t *kmem_hashmap, size_t entry_count) {
  void *data = kmem_calloc(entry_count * sizeof(kmem_hashmap_entry_t));
  if (!data)
    return false;
  kmem_hashmap->entries = (kmem_hashmap_entry_t *)data;
  kmem_hashmap->entry_count = entry_count;
  kmem_hashmap->items = 0;
  return true;
}

bool kmem_hashmap_set(kmem_hashmap_t *kmem_hashmap, void *key, void *val) {
  kmem_hashmap_entry_t *entry =
      (kmem_hashmap_entry_t *)kmem_hashmap_entry_get(kmem_hashmap, key);
  if (entry) {
    entry->key = key;
    entry->val = val;
    return true;
  }
  // key is not in kmem_hashmap
  size_t index = INDEX(key, kmem_hashmap->entry_count);
  entry = &kmem_hashmap->entries[index];
  while (true) {
    if (entry->next == NULL)
      break;
    entry = entry->next;
  }

  kmem_hashmap_entry_t *new_entry =
      (kmem_hashmap_entry_t *)kmem_malloc(sizeof(kmem_hashmap_entry_t));
  if (!new_entry)
    return false;

  new_entry->key = key;
  new_entry->val = val;
  new_entry->next = NULL;
  entry->next = new_entry;
  kmem_hashmap->items++;
  return true;
}

void *kmem_hashmap_get(kmem_hashmap_t *kmem_hashmap, void *key) {
  kmem_hashmap_entry_t *entry = kmem_hashmap_entry_get(kmem_hashmap, key);
  if (entry == NULL) {
    return 0;
  }
  return entry->val;
}

bool kmem_hashmap_remove(kmem_hashmap_t *kmem_hashmap, void *key) {
  size_t index = INDEX(key, kmem_hashmap->entry_count);
  kmem_hashmap_entry_t *entry = &kmem_hashmap->entries[index];
  kmem_hashmap_entry_t *prev = NULL;
  while (entry) {
    if (memcmp(entry->key, key, KM_KEY_LENGTH))
      break;
    prev = entry;
    entry = entry->next;
  }

  if (prev) {
    prev->next = entry->next;
    kmem_free(entry);
    kmem_free(entry->key);
  } else if (entry->next) {
    memcpy(entry, entry->next, sizeof(kmem_hashmap_entry_t));
    kmem_free(entry->next);
  }
  kmem_hashmap->items--;
  return true;
}
