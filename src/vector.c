#include <collections/vector.h>
#include <collections/memmove_safe.h>

struct vector_t {
    struct allocator_t allocator;
    ARR(struct vector_entry_t) data;
    copier_ft copy;
    size_t size;
    size_t capacity;
};

// Create
int create_vector_pro_ex(PTR(vector_t) dest, struct allocator_t allocator, copier_ft copier, size_t initial_capacity) {
    if (dest == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    if (allocator.malloc == nullptr || allocator.calloc == nullptr || allocator.realloc == nullptr || allocator.free == nullptr) {
        return INVALID_ALLOCATOR;
    }
    if (initial_capacity < 1) {
        return INVALID_ARGUMENT;
    }

    vector_t new_vector = allocator.malloc(sizeof(struct vector_t));
    if (new_vector == nullptr) {
        return CANT_ALLOCATE;
    }
    ARR(struct vector_entry_t) new_data = allocator.calloc(sizeof(struct vector_entry_t), initial_capacity);
    if (new_data == nullptr) {
        allocator.free(new_vector);
        return CANT_ALLOCATE;
    }
    new_vector->data = new_data;
    new_vector->allocator = allocator;
    new_vector->copy = copier;
    new_vector->size = 0;
    new_vector->capacity = initial_capacity;

    *dest = new_vector;

    return SUCCESS;
}

int copy_vector(PTR(vector_t) dest, vector_t src) {
    if (dest == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    if (src == nullptr) {
        *dest = nullptr;
        return SUCCESS;
    }
    if (src->copy == nullptr) {
        return UNSUPPORTED_OPERATION;
    }
    if (src->size == 0) {
        vector_t new_vector = nullptr;
        memory_t vector_mem = src->allocator.malloc(sizeof (struct vector_t));
        if (vector_mem == nullptr) {
            return CANT_ALLOCATE;
        }
        new_vector = vector_mem;
        new_vector->allocator = src->allocator;
        new_vector->capacity = 1;
        new_vector->size = 0;
        new_vector->copy = src->copy;
        memory_t data_vector = src->allocator.malloc(sizeof (struct vector_entry_t));
        if (data_vector == nullptr) {
            src->allocator.free(vector_mem);
            return CANT_ALLOCATE;
        }
        new_vector->data = data_vector;
        // These bits may be unnecessary
        new_vector->data->data = nullptr;
        new_vector->data->owns_data = false;
        *dest = new_vector;
        return SUCCESS;
    }

    vector_t new_vector = nullptr;
    int result = create_vector_pro_ex(&new_vector, src->allocator, src->copy, src->size);
    if (result != SUCCESS) {
        return result;
    }
    new_vector->size = src->size;

    for (size_t i = 0; i < src->size; i++) {
        struct vector_entry_t src_entry = src->data[i];
        struct vector_entry_t new_entry = {
            .data = nullptr,
            .owns_data = src_entry.data != nullptr
        };
        generic_t new_data_pointer = nullptr;
        int copy_result = src->copy(src_entry.data, src->allocator, &new_data_pointer);
        if (copy_result == SUCCESS) {
            new_entry.data = new_data_pointer;
            new_vector->data[i] = new_entry;
        } else { // Failure path
            for (size_t j = 0; j < i; j++) {
                struct vector_entry_t entry = new_vector->data[j];

                if (entry.owns_data && entry.data != nullptr) {
                    src->allocator.free(entry.data);
                }
            }
            src->allocator.free(new_vector->data);
            src->allocator.free(new_vector);
            return copy_result;
        }
    }
    *dest = new_vector;
    return SUCCESS;
}

// Read
int get_from_vector(vector_t src, size_t index, PTR(struct vector_entry_t) dest) {
    if (src == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    if (dest == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    if (index >= src->size) {
        return OUT_OF_BOUNDS;
    }
    *dest = src->data[index];
    return SUCCESS;
}

int get_size_of_vector(vector_t src, PTR(size_t) dest) {
    if (src == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    if (dest == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    *dest = src->size;
    return SUCCESS;
}

int get_capacity_of_vector(vector_t src, PTR(size_t) dest) {
    if (src == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    if (dest == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    *dest = src->capacity;
    return SUCCESS;
}

// Update
int put_in_vector(vector_t dest, size_t index, struct vector_entry_t src) {
    if (dest == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    if (index >= dest->size) {
        return OUT_OF_BOUNDS;
    }

    struct vector_entry_t old_entry = dest->data[index];

    if (src.data == nullptr) {
        src.owns_data = false;
    }

    if (old_entry.data == src.data) {
        dest->data[index].owns_data = src.owns_data;
        return SUCCESS;
    }

    if (old_entry.owns_data && old_entry.data != nullptr) {
        dest->allocator.free(old_entry.data);
    }

    dest->data[index] = src;
    return SUCCESS;
}

int putv_in_vector(vector_t dest, size_t dest_offset, size_t size, ARR(generic_t) src, size_t src_offset, bool transfer_ownership) {
    if (dest == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    if (src == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    if (dest_offset > dest->size || size > dest->size - dest_offset) {
        return OUT_OF_BOUNDS;
    }
    if (size == 0) {
        return SUCCESS;
    }
    for (size_t i = 0; i < size; i++) {
        struct vector_entry_t old_entry = dest->data[i + dest_offset];
        if (old_entry.owns_data) {
            dest->allocator.free(old_entry.data);
        }
        dest->data[i + dest_offset] = (struct vector_entry_t) {
            .owns_data = transfer_ownership && src[i + src_offset] != nullptr,
            .data = src[i + src_offset]
        };
    }
    return SUCCESS;
}

int append_to_vector(vector_t dest, generic_t src, bool owns_data) {
    if (dest == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    int result = reserve_vector(dest, dest->size + 1);
    if (result != SUCCESS) {
        return result;
    }
    owns_data &= src != nullptr;
    dest->data[dest->size] = (struct vector_entry_t) {
        .owns_data = owns_data,
        .data = src
    };
    dest->size++;
    return SUCCESS;
}

int appendv_to_vector(vector_t dest, size_t size, ARR(generic_t) src, size_t src_offset, bool transfer_ownership) {
    if (dest == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    int result = reserve_vector(dest, dest->size + size);
    if (result != SUCCESS) {
        return result;
    }
    for (size_t i = 0; i < size; i++) {
        dest->data[i + dest->size] = (struct vector_entry_t) {
            .owns_data = transfer_ownership && src[i + src_offset] != nullptr,
            .data = src[i + src_offset]
        };
    }
    dest->size += size;
    return SUCCESS;
}

int reserve_vector(vector_t dest, size_t new_minimum_capacity) {
    if (dest == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    if (dest->capacity >= new_minimum_capacity) {
        return SUCCESS;
    }
    memory_t new_memory = dest->allocator.realloc(dest->data, new_minimum_capacity * sizeof(struct vector_entry_t));
    if (new_memory == nullptr) {
        return CANT_ALLOCATE;
    }
    dest->data = new_memory;
    dest->capacity = new_minimum_capacity;
    return SUCCESS;
}

int shrink_vector(vector_t dest, size_t new_maximum_capacity) {
    if (dest == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    if (dest->size > new_maximum_capacity) {
        return INVALID_ARGUMENT;
    }
    if (new_maximum_capacity == 0) {
        return INVALID_ARGUMENT;
    }
    memory_t new_memory = dest->allocator.realloc(dest->data, new_maximum_capacity * sizeof(struct vector_entry_t));
    if (new_memory == nullptr) {
        return CANT_ALLOCATE;
    }
    dest->data = new_memory;
    dest->capacity = new_maximum_capacity;
    return SUCCESS;
}

// Delete
int remove_from_vector(vector_t src, size_t index, ARR(generic_t) dest, bool return_value) {
    if (src == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    if (return_value && dest == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    if (index >= src->size) {
        return OUT_OF_BOUNDS;
    }
    if (return_value) {
        *dest = src->data[index].data; // Transferring ownership
    } else {
        if (src->data[index].owns_data) {
            src->allocator.free(src->data[index].data);
        }
    }
    memmove_safe(src->data + index, src->data + index + 1, sizeof(struct vector_entry_t) * (src->size - index - 1));
    src->size--;

    src->data[src->size] = (struct vector_entry_t) {
        .data = nullptr,
        .owns_data = false
    };

    return SUCCESS;
}

int free_vector(PTR(vector_t) table) {
    if (table == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    if (*table == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }

    for (size_t i = 0; i < (*table)->size; i++) {
        struct vector_entry_t dest_entry = (*table)->data[i];
        if (dest_entry.owns_data && dest_entry.data != nullptr) {
            (*table)->allocator.free(dest_entry.data);
        }
    }

    (*table)->allocator.free((*table)->data);
    (*table)->allocator.free(*table);

    *table = nullptr;
    return SUCCESS;
}