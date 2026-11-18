#ifndef COLLECTIONS_HASH_TABLE_H
#define COLLECTIONS_HASH_TABLE_H

#include "types.h"

struct hash_bucket_t;
struct hash_table_t;
typedef PTR(struct hash_table_t) hash_table_t;

struct hash_table_entry_t {
    generic_t key;
    generic_t value;
    bool owns_key;
    bool owns_value;
};

typedef unsigned int (*hasher_ft)(generic_t);
unsigned int default_hasher(generic_t to_hash);

// Create
int create_hash_table_pro_ex(PTR(hash_table_t) new_table, unsigned int modulus, struct allocator_t allocator, hasher_ft hasher, comparator_ft key_comparator, copier_ft key_copier, copier_ft value_copier);
int create_hash_table(PTR(hash_table_t) new_table, unsigned int modulus);
int create_hash_table_pro(PTR(hash_table_t) new_table, unsigned int modulus, struct allocator_t allocator);
int create_hash_table_ex(PTR(hash_table_t) new_table, unsigned int modulus, hasher_ft hasher, comparator_ft key_comparator, copier_ft key_copier, copier_ft value_copier);

// Read
int get_value_from_hash_table(hash_table_t src, generic_t key, PTR(generic_t) dest);
int copy_hash_table(hash_table_t src, PTR(hash_table_t) dest);

// Update
int put_value_in_hash_table(hash_table_t dest, generic_t key, generic_t value, bool owns_key, bool owns_value);
int rehash_hash_table(hash_table_t table, unsigned int new_modulus);

// Delete
int remove_value_from_hash_table(hash_table_t src, generic_t key, PTR(struct hash_table_entry_t) dest, bool return_value);
int free_hash_table(PTR(hash_table_t) table);

#endif //COLLECTIONS_HASH_TABLE_H
