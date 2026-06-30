#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Si este .c define ALLOCATE_SHMEM, se crean las variables.
// Si no lo define, se tratan como extern automáticos.
#ifdef ALLOCATE_SHMEM
  #define SHMEM_EXTD
  #define SHMEM_INITD(x) = x
#else
  #define SHMEM_EXTD extern
  #define SHMEM_INITD(x)
#endif

SHMEM_EXTD bool initialized SHMEM_INITD(false);

#ifdef __cplusplus
}
#endif
