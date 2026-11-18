#include <collections/types.h>

const_string_t const error_messages[ENUM_ERROR_T_SENTINEL_THAT_IS_ALSO_SIZE_OF_ENUM] = {
    "SUCCESS",
    "NULL_POINTER_ENCOUNTERED",
    "CANT_ALLOCATE",
    "INVALID_ARGUMENT",
    "OUT_OF_BOUNDS",
    "INVALID_ENUM",
    "UNSUPPORTED_OPERATION",
    "INVALID_ALLOCATOR"
};

const struct allocator_t default_allocator = {
    .malloc = malloc,
    .calloc = calloc,
    .realloc = realloc,
    .free = free
};

int default_comparator(generic_t a, generic_t b) {
    if (a == b) return 0;
    return a < b ? -1 : 1;
}