#ifndef COLLECTIONS_BUFFER_H
#define COLLECTIONS_BUFFER_H

#include "types.h"

#ifdef __cplusplus
    #define EXTERN_C_BEGIN extern "C" {
    #define EXTERN_C_END }
#else
    #define EXTERN_C_BEGIN
    #define EXTERN_C_END
#endif

#define GENERIC_BUFFER_DECLARATION(prefix, type) \
EXTERN_C_BEGIN \
struct prefix##buffer_t; \
typedef PTR(struct prefix##buffer_t) prefix##buffer_t; \
int create_##prefix##buffer(PTR(prefix##buffer_t) dest, size_t initial_capacity); \
int create_##prefix##buffer_ex(PTR(prefix##buffer_t) dest, size_t initial_capacity, struct allocator_t allocator); \
int get_from_##prefix##buffer(prefix##buffer_t src, int index, PTR(type) dest); \
int getv_from_##prefix##buffer(prefix##buffer_t src, int index, int size, ARR(type) dest, int offset); \
int get_##prefix##buffer_size(prefix##buffer_t src, PTR(size_t) dest); \
int get_##prefix##buffer_capacity(prefix##buffer_t src, PTR(size_t) dest); \
int get_##prefix##buffer_ptr(prefix##buffer_t src, PTR(type*) dest); \
int put_into_##prefix##buffer(prefix##buffer_t dest, size_t index, type src); \
int putv_into_##prefix##buffer(prefix##buffer_t dest, size_t dest_offset, size_t size, const ARR(type) src, size_t offset); \
int copy_from_##prefix##buffer(prefix##buffer_t src, size_t src_offset, prefix##buffer_t dest, size_t dest_offset, size_t size); \
int clear_##prefix##buffer(prefix##buffer_t src); \
int clear_##prefix##buffer_ex(prefix##buffer_t src, type value); \
int shrink_##prefix##buffer(prefix##buffer_t src, int new_maximum_capacity); \
int reserve_##prefix##buffer(prefix##buffer_t src, int new_minimum_capacity); \
int free_##prefix##buffer(PTR(prefix##buffer_t) src); \
EXTERN_C_END

#define GENERIC_BUFFER_DEFINITION(prefix, type, zero_value) \
    struct prefix##buffer_t { \
    struct allocator_t allocator; \
    type* data; \
    size_t size; \
    size_t capacity; \
}; \
int create_##prefix##buffer(PTR(prefix##buffer_t) dest, size_t initial_capacity) { \
    return create_##prefix##buffer_ex(dest, initial_capacity, default_allocator); \
} \
int create_##prefix##buffer_ex(PTR(prefix##buffer_t) dest, size_t initial_capacity, struct allocator_t allocator) { \
    if (initial_capacity == 0) { \
        return INVALID_ARGUMENT; \
    } \
    if (allocator.malloc == nullptr || allocator.calloc == nullptr || allocator.realloc == nullptr || allocator.free == nullptr) { \
        return INVALID_ARGUMENT; \
    } \
    if (dest == nullptr) { \
        return NULL_POINTER_ENCOUNTERED; \
    } \
    *dest = allocator.calloc(sizeof(struct prefix##buffer_t), 1);\
    if (*dest == nullptr) {\
        return CANT_ALLOCATE; \
    }\
    (*dest)->allocator = allocator; \
    (*dest)->size = 0; \
    (*dest)->capacity = initial_capacity; \
    memory_t new_buffer = allocator.calloc(sizeof(type), initial_capacity); \
    if (new_buffer == nullptr) { \
        allocator.free(*dest); \
        return CANT_ALLOCATE; \
    } \
    (*dest)->data = new_buffer; \
    return SUCCESS; \
} \
int get_from_##prefix##buffer(prefix##buffer_t src, int index, PTR(type) dest) { \
    if (src == nullptr) { \
        return NULL_POINTER_ENCOUNTERED; \
    } \
    if (dest == nullptr) { \
        return NULL_POINTER_ENCOUNTERED; \
    } \
    if (index < 0 || index >= src->size) { \
        return OUT_OF_BOUNDS; \
    } \
    *dest = src->data[index]; \
    return SUCCESS; \
} \
int getv_from_##prefix##buffer(prefix##buffer_t src, int index, int size, ARR(type) dest, int offset) { \
    if (src == nullptr || dest == nullptr) { \
        return NULL_POINTER_ENCOUNTERED; \
    } \
    if (index < 0 || size < 0 || offset < 0) { \
        return INVALID_ARGUMENT; \
    } \
    if (src->size < index + size) { \
        return OUT_OF_BOUNDS; \
    } \
    if (size == 0) { \
        return SUCCESS; \
    } \
    memmove_safe(dest + offset, src->data + index, (size_t)size * sizeof(type)); \
    return SUCCESS; \
} \
int get_##prefix##buffer_size(prefix##buffer_t src, PTR(size_t) dest) { \
    if (src == nullptr) { \
        return NULL_POINTER_ENCOUNTERED; \
    } \
     if (dest == nullptr) { \
        return NULL_POINTER_ENCOUNTERED; \
    } \
    *dest = src->size; \
    return SUCCESS; \
} \
int get_##prefix##buffer_capacity(prefix##buffer_t src, PTR(size_t) dest) { \
    if (src == nullptr) { \
        return NULL_POINTER_ENCOUNTERED; \
    } \
     if (dest == nullptr) { \
        return NULL_POINTER_ENCOUNTERED; \
    } \
    *dest = src->capacity; \
    return SUCCESS; \
} \
int get_##prefix##buffer_ptr(prefix##buffer_t src, PTR(type*) dest) { \
    if (src == nullptr) { \
        return NULL_POINTER_ENCOUNTERED; \
    } \
     if (dest == nullptr) { \
        return NULL_POINTER_ENCOUNTERED; \
    } \
    *dest = src->data; \
    return SUCCESS; \
} \
int put_into_##prefix##buffer(prefix##buffer_t dest, size_t index, type src) { \
    if (dest == nullptr) { \
        return NULL_POINTER_ENCOUNTERED; \
    } \
    if (index >= dest->size) { \
        return OUT_OF_BOUNDS; \
    } \
    dest->data[index] = src; \
    return SUCCESS; \
} \
int putv_into_##prefix##buffer(prefix##buffer_t dest, size_t dest_offset, size_t size, const ARR(type) src, size_t offset) { \
    if (src == nullptr) { \
        return NULL_POINTER_ENCOUNTERED; \
    } \
    if (dest->size < size + dest_offset) { \
        return OUT_OF_BOUNDS; \
    } \
    memmove_safe(dest->data + dest_offset, src + offset, size * sizeof(type)); \
    return SUCCESS; \
} \
int copy_from_##prefix##buffer(prefix##buffer_t src, size_t src_offset, prefix##buffer_t dest, size_t dest_offset, size_t size) { \
    if (src == nullptr || dest == nullptr) { \
        return NULL_POINTER_ENCOUNTERED; \
    } \
    if (src->size < size + src_offset || dest->size < size + dest_offset) { \
        return OUT_OF_BOUNDS; \
    } \
    memmove_safe(dest->data + dest_offset, src->data + src_offset, size * sizeof(type)); \
    return SUCCESS; \
} \
int clear_##prefix##buffer(prefix##buffer_t src) { \
    return clear_##prefix##buffer_ex(src, zero_value); \
} \
int clear_##prefix##buffer_ex(prefix##buffer_t src, type value) { \
    if (src == nullptr) { \
        return NULL_POINTER_ENCOUNTERED; \
    } \
    for (size_t i = 0; i < src->size; i++) { \
        src->data[i] = value; \
    } \
    return SUCCESS; \
} \
int shrink_##prefix##buffer(prefix##buffer_t src, int new_maximum_capacity) { \
    if (src == nullptr) { \
        return NULL_POINTER_ENCOUNTERED; \
    } \
    if (new_maximum_capacity > src->capacity) { \
        return SUCCESS; \
    } \
    if (new_maximum_capacity < src->size) {\
        return INVALID_ARGUMENT; \
    } \
    memory_t new_data = src->allocator.realloc(src->data, new_maximum_capacity * sizeof(type)); \
    if (new_data) { \
        src->data = new_data; \
        src->capacity = new_maximum_capacity; \
        return SUCCESS; \
    } \
    return CANT_ALLOCATE; \
} \
int reserve_##prefix##buffer(prefix##buffer_t src, int new_minimum_capacity) { \
    if (src == nullptr) { \
        return NULL_POINTER_ENCOUNTERED; \
    } \
    if (new_minimum_capacity <= src->capacity) { \
        return SUCCESS; \
    } \
    memory_t new_data = src->allocator.realloc(src->data, new_minimum_capacity * sizeof(type)); \
    if (new_data) { \
        src->data = new_data; \
        src->capacity = new_minimum_capacity; \
        return SUCCESS; \
    } \
    return CANT_ALLOCATE; \
} \
int free_##prefix##buffer(PTR(prefix##buffer_t) src) { \
    if (src == nullptr) { \
        return NULL_POINTER_ENCOUNTERED; \
    } \
    if (*src == nullptr) { \
        return NULL_POINTER_ENCOUNTERED; \
    } \
    if ((*src)->data != nullptr) { \
        (*src)->allocator.free((*src)->data); \
    } \
    (*src)->allocator.free(*src); \
    *src = nullptr; \
    return SUCCESS; \
}

GENERIC_BUFFER_DECLARATION(i, int)
GENERIC_BUFFER_DECLARATION(ui, unsigned int)
GENERIC_BUFFER_DECLARATION(f, float)
GENERIC_BUFFER_DECLARATION(b, unsigned char)
GENERIC_BUFFER_DECLARATION(d, double)
GENERIC_BUFFER_DECLARATION(l, long long int)
GENERIC_BUFFER_DECLARATION(s, short int)
GENERIC_BUFFER_DECLARATION(ptr, memory_t)
GENERIC_BUFFER_DECLARATION(c, char)
GENERIC_BUFFER_DECLARATION(ul, unsigned long long int)
GENERIC_BUFFER_DECLARATION(us, unsigned short int)
GENERIC_BUFFER_DECLARATION(z, size_t)
GENERIC_BUFFER_DECLARATION(str, char *)

#endif //COLLECTIONS_BUFFER_H
