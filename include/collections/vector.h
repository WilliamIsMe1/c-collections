#ifndef COLLECTIONS_VECTOR_H
#define COLLECTIONS_VECTOR_H

#include "types.h"

struct vector_t;
typedef PTR(struct vector_t) vector_t;

struct vector_entry_t {
    generic_t data;
    bool owns_data;
};

// Create
int create_vector_pro_ex(PTR(vector_t) dest, struct allocator_t allocator, copier_ft copier, size_t initial_capacity);
inline int create_vector(PTR(vector_t) dest) {return create_vector_pro_ex(dest, default_allocator, nullptr, 10);}
inline int create_vector_pro(PTR(vector_t) dest, const struct allocator_t allocator, const copier_ft copier) {return create_vector_pro_ex(dest, allocator, copier, 10);}
inline int create_vector_ex(PTR(vector_t) dest, const size_t initial_capacity) {return create_vector_pro_ex(dest, default_allocator, nullptr, initial_capacity);}
int copy_vector(PTR(vector_t) dest, vector_t src);

// Read
int get_from_vector(vector_t src, size_t index, PTR(struct vector_entry_t) dest);
int get_size_of_vector(vector_t src, PTR(size_t) dest);
int get_capacity_of_vector(vector_t src, PTR(size_t) dest);

// Update
int put_in_vector(vector_t dest, size_t index, struct vector_entry_t src);
int putv_in_vector(vector_t dest, size_t dest_offset, size_t size, ARR(generic_t) src, size_t src_offset, bool transfer_ownership); // Bulk operations here make me question the effectiveness of the current ownership system
int append_to_vector(vector_t dest, generic_t src, bool owns_data);
int appendv_to_vector(vector_t dest, size_t size, ARR(generic_t) src, size_t src_offset, bool transfer_ownership);
int reserve_vector(vector_t dest, size_t new_minimum_capacity);
int shrink_vector(vector_t dest, size_t new_maximum_capacity);

// Delete
int remove_from_vector(vector_t src, size_t index, ARR(generic_t) dest, bool return_value);
int free_vector(PTR(vector_t) table);

#endif //COLLECTIONS_VECTOR_H
