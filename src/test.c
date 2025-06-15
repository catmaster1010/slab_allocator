#include "common.h"
#include "lib/hashmap.h"
#include "slab.h"

struct foo {
  int a;
  int b;
  int c;
};

void foo_constructor(void *foo_ptr) {
  struct foo *fp = (struct foo *)foo_ptr;
  fp->a = 54321;
  fp->b = 0xC0FFEE;
  fp->c = 0;
  return;
}

void static inline print_cache_contents(kmem_cache_t *cp) {
  kmem_debug("---------------------------\n");
  kmem_debug("CACHE DATA\n");
  kmem_debug("---------------------------\n");
  kmem_debug("slabs_partial : %p\n", cp->slabs_partial);
  kmem_debug("slabs_free : %p\n", cp->slabs_free);
  kmem_debug("name : %s\n", cp->name);
  kmem_debug("object_size : %lu\n", cp->object_size);
  kmem_debug("align : %lu\n", cp->align);
  kmem_debug("slabs : %p\n", cp->slabs);
  kmem_debug("constructor : %p\n", cp->constructor);
  kmem_debug("destructor : %p\n", cp->destructor);
  kmem_debug("num_active : %lu\n", cp->num_active);
  kmem_debug("num_allocations : %lu\n", cp->num_allocations);
  kmem_debug("hashmap : %p\n", cp->hashmap);
  kmem_debug("---------------------------\n");
}
int main() {

  kmem_cache_t *cp = kmem_cache_create("foo_cache", sizeof(struct foo), 0,
                                       foo_constructor, NULL);
  kmem_debug("New cache created!\n");

  print_cache_contents(cp);
  struct foo *new_foo = (struct foo *)kmem_cache_alloc(cp, KM_NOSLEEP);
  struct foo *new_foo2 = (struct foo *)kmem_cache_alloc(cp, KM_NOSLEEP);
  kmem_debug("Allocations made!\n");
  print_cache_contents(cp);
  assert(cp->num_allocations == 2);

  kmem_debug("`new_foo` DATA\n");
  kmem_debug("---------------------------\n");
  kmem_debug("a : %d\n", new_foo->a);
  kmem_debug("a : %0x\n", new_foo->b);
  kmem_debug("a : %d\n", new_foo->c);
  kmem_debug("---------------------------\n");

  kmem_assert(new_foo->a == 54321);
  kmem_assert(new_foo->b == 0xC0FFEE);
  kmem_assert(new_foo->c == 0);

  kmem_cache_free(cp, new_foo2);
  assert(cp->num_allocations == 1);
  kmem_cache_free(cp, new_foo);
  assert(cp->num_allocations == 0);
  print_cache_contents(cp);

  kmem_debug("Allocations Free!\n");

  kmem_debug("\n\n");

  kmem_debug("TESTING HASHMAP\n");
  kmem_hashmap_t *kmem_hashmap =
      (kmem_hashmap_t *)malloc(sizeof(kmem_hashmap_t));
  kmem_hashmap_create(kmem_hashmap, 64);

  int b = 123454321;
  kmem_debug("Value of b : %d\n", b);
  kmem_hashmap_set(kmem_hashmap, (void *)"h", &b);
  kmem_debug("set \"h\" to `b` \n");
  int *a = (int *)kmem_hashmap_get(kmem_hashmap, (void *)"h");
  kmem_debug("Got value of b.\nValue of b : %d\n", *a);
  assert(*a == b);

  return 0;
}
