#include "lib/hashmap.h"
#include "slab.h"

struct foo {
  int a;
};

void foo_constructor(void *foo_ptr) {
  struct foo *fp = (struct foo *)foo_ptr;
  fp->a = 54321;
  return;
}

int main() {

  kmem_cache_t *cp = kmem_cache_create("kmem_cache", sizeof(struct foo), 0,
                                       foo_constructor, NULL);
  kmem_printf("CACHE RECIVED! %p", cp);
  kmem_printf("CONTENTS: %s, %lu", cp->name, cp->num_active);

  struct foo *new_foo = (struct foo *)kmem_cache_alloc(cp, KM_NOSLEEP);
  kmem_cache_free(cp, new_foo);

  kmem_hashmap_t *kmem_hashmap =
      (kmem_hashmap_t *)malloc(sizeof(kmem_hashmap_t));
  kmem_hashmap_create(kmem_hashmap, 64);
  int *a;
  int b = 123454321;
  kmem_hashmap_set(kmem_hashmap, (void *)"h", &b);
  int *c = (int *)kmem_hashmap_get(kmem_hashmap, (void *)"h");
  printf("%d", *c);

  return 0;
}
