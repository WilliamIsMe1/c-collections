#ifndef COLLECTIONS_TYPES_H
#define COLLECTIONS_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

typedef enum {
    SUCCESS,
    NULL_POINTER_ENCOUNTERED,
    CANT_ALLOCATE,
    INVALID_ARGUMENT,
    OUT_OF_BOUNDS,
    INVALID_ENUM,
    UNSUPPORTED_OPERATION,
    INVALID_ALLOCATOR,
    ENUM_ERROR_T_SENTINEL_THAT_IS_ALSO_SIZE_OF_ENUM // DON'T USE AS AN ERROR CODE; it's for extending the error_t
} error_t;

#define PTR(type) type *
#define CONST_PTR(type) const type *
#define PTR_CONST(type) type * const
#define CONST_PTR_CONST(type) const type * const

#define ARR(type) type *

typedef ARR(char) string_t;
typedef CONST_PTR_CONST(char) const_string_t;

extern const_string_t const error_messages[ENUM_ERROR_T_SENTINEL_THAT_IS_ALSO_SIZE_OF_ENUM]; // EUUGH descriptive names can be UGLY sometimes

typedef void* generic_t;
typedef void* memory_t;

static inline bool is_power_of_two(const unsigned int value) {
    return value != 0 && (value & value - 1) == 0;
}

struct allocator_t {
    memory_t (*malloc ) (size_t len              );
    memory_t (*calloc ) (size_t size,  size_t len);
    memory_t (*realloc) (memory_t ptr, size_t len);
    void     (*free   ) (memory_t ptr            );
};

const extern struct allocator_t default_allocator;

typedef int (*comparator_ft)(generic_t, generic_t);
typedef int (*copier_ft)(generic_t,struct allocator_t,PTR(generic_t) return_val);

int default_comparator(generic_t a, generic_t b);

#ifdef __cplusplus
}
#endif

#endif
