#ifndef COMMON_H
#define COMMON_H
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define kmem_printf printf
#define kmem_debug printf
#define kmem_sleep sleep
#define PAGE_SZ sysconf(_SC_PAGE_SIZE)
#define kmem_malloc malloc
#define kmem_free free
#define kmem_posix_memalign posix_memalign
#define kmem_assert assert
#define kmem_calloc(x) calloc(x, x)
#endif
