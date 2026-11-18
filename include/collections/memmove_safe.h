//
// Created by William Bailey on 11/7/26.
//

#ifndef COLLECTIONS_MEMMOVE_SAFE_H
#define COLLECTIONS_MEMMOVE_SAFE_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void *memmove_safe(void *dest, const void *src, size_t n);

#ifdef __cplusplus
}
#endif

#endif //COLLECTIONS_MEMMOVE_SAFE_H
