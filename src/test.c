#include "slab.h"
#include <stdio.h>
#include <stdlib.h>
int main() {
  kmem_init(printf, posix_memalign);
  return 0;
}
