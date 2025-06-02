#include "lib/hashmap.h"
#include "slab.h"
struct foo {
  int a;
};
int main() {

  kmem_cache_t *cp =
      kmem_cache_create("kmem_cache", sizeof(struct foo), 0, NULL, NULL);
  kmem_printf("CACHE RECIVED! %p", cp);
  kmem_printf("CONTESTS: %s, %lu", cp->name, cp->num_active);
  kmem_cache_alloc(cp, KM_NOSLEEP);

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
