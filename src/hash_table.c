#include <collections/hash_table.h>

#include <string.h>

// TODO LIST:
// - Serialization
// - Iteration
// - Maybe switching to tree instead of linked list

unsigned int default_hasher(const generic_t to_hash) {
    const unsigned long int addr = (unsigned long int) to_hash;
    return (unsigned int)(addr ^ (addr >> 32)); // XOR fold for preventing collisions
}

struct hash_bucket_t {
    PTR(struct hash_bucket_t) next;
    generic_t key;
    generic_t value;
    bool owns_key;
    bool owns_value;
};

struct hash_table_t {
    hasher_ft hasher;
    comparator_ft comparator;
    copier_ft key_copier;
    copier_ft value_copier;
    struct allocator_t allocator;
    ARR(PTR(struct hash_bucket_t)) buckets;
    unsigned int modulus;
    unsigned int record_count;
};

int create_hash_table_pro_ex(PTR(hash_table_t) new_table, const unsigned int modulus, const struct allocator_t allocator, const hasher_ft hasher, const comparator_ft key_comparator, const copier_ft key_copier, const copier_ft value_copier) {
    if (!is_power_of_two(modulus)) {
        return INVALID_ARGUMENT;
    }
    if (allocator.malloc == nullptr || allocator.calloc == nullptr || allocator.realloc == nullptr || allocator.free == nullptr) {
        return INVALID_ALLOCATOR;
    }
    if (hasher == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    if (key_comparator == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    hash_table_t new_table_mem = allocator.calloc(sizeof(struct hash_table_t), 1);
    if (new_table_mem == NULL) {
        return CANT_ALLOCATE;
    }
    new_table_mem->buckets = allocator.calloc(sizeof(struct hash_bucket_t*), modulus);
    if (new_table_mem->buckets == NULL) {
        allocator.free(new_table_mem);
        return CANT_ALLOCATE;
    }
    new_table_mem->modulus = modulus;
    new_table_mem->hasher = hasher;
    new_table_mem->allocator = allocator;
    new_table_mem->comparator = key_comparator;
    new_table_mem->key_copier = key_copier; // NULLABLE
    new_table_mem->value_copier = value_copier; // NULLABLE
    new_table_mem->record_count = 0;

    *new_table = new_table_mem;

    return SUCCESS;
}

int create_hash_table(PTR(hash_table_t) new_table, const unsigned int modulus) {
    return create_hash_table_pro_ex(new_table, modulus, default_allocator, default_hasher, default_comparator, nullptr, nullptr);
}

int create_hash_table_pro(PTR(hash_table_t) new_table, const unsigned int modulus, const struct allocator_t allocator) {
    return create_hash_table_pro_ex(new_table, modulus, allocator, default_hasher, default_comparator, nullptr, nullptr);
}

int create_hash_table_ex(PTR(hash_table_t) new_table, const unsigned int modulus, const hasher_ft hasher, const comparator_ft key_comparator, const copier_ft key_copier, const copier_ft value_copier) {
    return create_hash_table_pro_ex(new_table, modulus, default_allocator, hasher, key_comparator, key_copier, value_copier);
}

int get_value_from_hash_table(const hash_table_t src, const generic_t key, PTR(generic_t) dest) {
    if (src == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }

    const unsigned int hash_value = src->hasher(key) % src->modulus;
    PTR(struct hash_bucket_t) current_search_point = src->buckets[hash_value];
    while (current_search_point != nullptr && src->comparator(key, current_search_point->key) != 0) {
        current_search_point = current_search_point->next;
    }
    if (current_search_point != nullptr) {
        *dest = current_search_point->value;
        return SUCCESS;
    }

    return OUT_OF_BOUNDS;
}

static void free_bucket_chain(PTR(struct hash_bucket_t), struct allocator_t);

int copy_hash_table(const hash_table_t src, PTR(hash_table_t) dest)
{
    if (dest == nullptr) return NULL_POINTER_ENCOUNTERED;
    if (src == nullptr) {
        *dest = nullptr;
        return SUCCESS;
    }
    if (src->key_copier == nullptr || src->value_copier == nullptr)
        return UNSUPPORTED_OPERATION;

    hash_table_t new_table = nullptr;
    int result = create_hash_table_pro_ex(&new_table, src->modulus, src->allocator, src->hasher, src->comparator, src->key_copier, src->value_copier);
    if (result != SUCCESS) return result;

    for (unsigned int i = 0; i < src->modulus; i++) {
        PTR(struct hash_bucket_t) old = src->buckets[i];
        PTR(PTR(struct hash_bucket_t)) slot = &new_table->buckets[i];

        while (old != nullptr) {
            PTR(struct hash_bucket_t) node =
                src->allocator.calloc(sizeof(struct hash_bucket_t), 1);
            if (node == nullptr) {
                result = CANT_ALLOCATE;
                goto copy_fail;
            }

            *slot = node;

            result = src->key_copier(old->key, src->allocator, &node->key);
            if (result != SUCCESS) {
                if (node->key != nullptr) src->allocator.free(node->key);
                goto copy_fail;
            }
            node->owns_key = old->key != nullptr;

            result = src->value_copier(old->value, src->allocator, &node->value);
            if (result != SUCCESS) {
                if (node->owns_key && node->key) src->allocator.free(node->key);
                if (node->value != nullptr) src->allocator.free(node->value);
                goto copy_fail;
            }
            node->owns_value = old->value != nullptr;

            slot = &node->next;
            old  = old->next;
        }
    }

    new_table->record_count = src->record_count;
    *dest = new_table;
    return SUCCESS;

copy_fail:
    free_hash_table(&new_table);
    return result;
}

static void free_bucket_chain(PTR(struct hash_bucket_t) bucket, const struct allocator_t allocator) {
    if (bucket == nullptr) return;

    if (bucket->key != nullptr && bucket->owns_key) {
        allocator.free(bucket->key);
    }
    if (bucket->value != nullptr && bucket->owns_value) {
        allocator.free(bucket->value);
    }

    PTR(struct hash_bucket_t) current = bucket->next;
    while (current != nullptr) {
        PTR(struct hash_bucket_t) next = current->next;

        if (current->key != nullptr && current->owns_key) {
            allocator.free(current->key);
        }
        if (current->value != nullptr && current->owns_value) {
            allocator.free(current->value);
        }
        allocator.free(current);

        current = next;
    }
    allocator.free(bucket);
}

int put_value_in_hash_table(hash_table_t dest, generic_t key, generic_t value, bool owns_key, bool owns_value) {
    if (dest == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }

    owns_key &= key != nullptr; // NEVER claim ownership of a nullptr
    owns_value &= value != nullptr; // NEVER claim ownership of a nullptr

    const unsigned int hash_value = dest->hasher(key) % dest->modulus;
    PTR(PTR(struct hash_bucket_t)) prev = &dest->buckets[hash_value];
    PTR(struct hash_bucket_t) placement_point = dest->buckets[hash_value];
    bool do_record = true;

    while (true) {
        if (placement_point == nullptr) {
            *prev = dest->allocator.calloc(sizeof(struct hash_bucket_t), 1);
            if (*prev == nullptr) return CANT_ALLOCATE;
            placement_point = *prev;
            break;
        }
        if (dest->comparator(placement_point->key, key) == 0) {
            do_record = false;
            if (placement_point->owns_key && placement_point->key != nullptr) {
                dest->allocator.free(placement_point->key);
            }
            if (placement_point->owns_value && placement_point->value != nullptr) {
                dest->allocator.free(placement_point->value);
            }
            break;
        }
        prev = &placement_point->next;
        placement_point = placement_point->next;
    }

    placement_point->key = key;
    placement_point->owns_key = owns_key;
    placement_point->value = value;
    placement_point->owns_value = owns_value;
    if (do_record) dest->record_count++;
    return SUCCESS;
}

int rehash_hash_table(hash_table_t table, unsigned int new_modulus) {
    if (table == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }
    if (!is_power_of_two(new_modulus)) {
        return INVALID_ARGUMENT;
    }

    ARR(PTR(struct hash_bucket_t)) all_buckets = table->allocator.calloc(sizeof(void*), table->record_count);
    if (all_buckets == nullptr) {
        return CANT_ALLOCATE;
    }

    int idx = 0;

    for (int i = 0; i < table->modulus; i++) {
        PTR(struct hash_bucket_t) current_bucket = table->buckets[i];
        while (current_bucket != nullptr) {
            all_buckets[idx] = current_bucket;
            idx++;
            current_bucket = current_bucket->next;
        }
    }

    memory_t new_mem = table->allocator.realloc(table->buckets, sizeof(PTR(struct hash_bucket_t)) * new_modulus);
    if (new_mem == nullptr) {
        table->allocator.free(all_buckets);
        return CANT_ALLOCATE;
    }
    table->buckets = new_mem;
    memset(table->buckets, 0,
       sizeof(*table->buckets) * new_modulus);

    for (int i = 0; i < table->record_count; i++) {
        PTR(struct hash_bucket_t) bucket = all_buckets[i];
        int hash = table->hasher(bucket->key);
        PTR(PTR(struct hash_bucket_t)) placement_point = &table->buckets[hash % new_modulus];
        while (*placement_point != nullptr) {
            placement_point = &(*placement_point)->next;
        }
        *placement_point = bucket; // placing :D
        bucket->next = nullptr; // Safety
    }

    table->modulus = new_modulus;

    table->allocator.free(all_buckets);
    return SUCCESS;
}


int remove_value_from_hash_table( hash_table_t src, generic_t key, PTR(struct hash_table_entry_t) dest, bool return_value) {
    if (src == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }

    if (return_value && dest == nullptr) {
        return NULL_POINTER_ENCOUNTERED;
    }

    const unsigned int hash_value = src->hasher(key) % src->modulus;

    PTR(PTR(struct hash_bucket_t)) prev = &src->buckets[hash_value];

    PTR(struct hash_bucket_t) current = *prev;

    while (current != nullptr && src->comparator(key, current->key) != 0) {
        prev = &current->next;
        current = current->next;
    }
    if (current == nullptr) {
        return OUT_OF_BOUNDS;
    }

    *prev = current->next;

    if (return_value) {
        dest->key = current->key;
        dest->value = current->value;
        dest->owns_key = current->owns_key;
        dest->owns_value = current->owns_value;
    } else {
        if (current->owns_key && current->key != nullptr) {
            src->allocator.free(current->key);
        }

        if (current->owns_value && current->value != nullptr) {
            src->allocator.free(current->value);
        }
    }

    src->allocator.free(current);
    src->record_count--;

    return SUCCESS;
}


int free_hash_table(PTR(hash_table_t) table) {
    if (table == nullptr) return NULL_POINTER_ENCOUNTERED;
    if (*table == nullptr) return NULL_POINTER_ENCOUNTERED;
    hash_table_t src = *table;
    for (int i = 0; i < src->modulus; i++) {
        free_bucket_chain(src->buckets[i], src->allocator);
    }
    src->allocator.free(src->buckets);
    src->allocator.free(src);
    *table = nullptr;
    return SUCCESS;
}