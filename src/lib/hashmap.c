#include "hashmap.h"
#include "../slab.h"
#include "string.h"

// http://www.cse.yorku.ca/~oz/hash.html
static inline size_t hash(char *key, size_t size) {
  size_t hash = 5381;
  for (size_t i = 0; i < size; i++) {
    uint8_t c = key[i];
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }
  return hash;
}

#define INDEX(KEY, ENTRY_COUNT) {hash(KEY, strlen(KEY)) % ENTRY_COUNT}

static inline kmem_hashmap_entry_t *
kmem_hashmap_entry_get(kmem_hashmap_t *kmem_hashmap, char *key) {
  size_t index = INDEX(key, kmem_hashmap->entry_count);
  kmem_hashmap_entry_t *entry = &kmem_hashmap->entries[index];
  size_t key_length = strlen(key);
  while (entry) {
    if (key_length == entry->key_length) {
      if (!memcmp(entry->key, key, key_length))
        break;
    }
    entry = entry->next;
  }
  return entry;
}

bool kmem_hashmap_create(kmem_hashmap_t *kmem_hashmap, size_t entry_count) {
  void *data = kmem_malloc(entry_count * sizeof(kmem_hashmap_entry_t));
  if (!data)
    return false;
  kmem_hashmap->entries = (kmem_hashmap_entry_t *)data;
  kmem_hashmap->entry_count = entry_count;
  kmem_hashmap->items = 0;
  return true;
}

bool kmem_hashmap_set(kmem_hashmap_t *kmem_hashmap, char *key, void *val) {
  kmem_hashmap_entry_t *entry =
      (kmem_hashmap_entry_t *)kmem_hashmap_entry_get(kmem_hashmap, key);
  if (entry) {
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
  new_entry->key_length = strlen(key);
  new_entry->val = val;
  new_entry->next = NULL;
  entry->next = new_entry;
  kmem_hashmap->items++;
  return true;
}

void *kmem_hashmap_get(kmem_hashmap_t *kmem_hashmap, char *key) {
  kmem_hashmap_entry_t *entry = kmem_hashmap_entry_get(kmem_hashmap, key);
  if (entry == NULL) {
    return 0;
  }
  return entry->val;
}

bool kmem_hashmap_remove(kmem_hashmap_t *kmem_hashmap, char *key) {
  size_t index = INDEX(key, kmem_hashmap->entry_count);
  kmem_hashmap_entry_t *entry = &kmem_hashmap->entries[index];
  kmem_hashmap_entry_t *prev = NULL;
  size_t key_length = strlen(key);
  while (entry) {
    if (key_length == entry->key_length) {
      if (memcmp(entry->key, key, key_length))
        break;
    }
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
