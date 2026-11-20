#ifndef COLLECTIONS_HASH_TABLE_H
#define COLLECTIONS_HASH_TABLE_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct hash_bucket_t;
/**
 * @brief Associative array (hash table) mapping generic keys to generic values
 *
 * Maps keys to values using hashing. Average-case lookup, insertion, and deletion are expected to be
 * O(1); worst case is O(n) under heavy collisions.
 *
 * The table can be customized through the use of custom allocators or custom hash functions,
 * comparison functions, and copier functions. Inclusion of the copy operation is optional, and
 * instead makes the function unavailable.
 *
 * Ownership of pointers is decided on a case by case basis, with the unique limitation that the same
 * allocator must be used for all entries AND the hash table, except in cases where the table does
 * not own the pointers. You allocate a pointer, pass it to the table, and transfer ownership so that
 * when the table is freed using `free_hash_table`, no memory leaks from inside the table, except
 * what the user of the library has failed to transfer.
 *
 * Any table created by `copy_hash_table` owns every single key-value pair that was present in the
 * old table, since there is no real tracking of these variables on the outside.
 *
 * @note Not thread safe.
 *
 * @see struct allocator_t
 * @see create_hash_table
 * @see create_hash_table_pro
 * @see create_hash_table_ex
 * @see create_hash_table_pro_ex
 *
 * @author William Bailey
 */
struct hash_table_t;
/**
 * @brief Handle to the opaque struct that is `struct hash_table_t`
 *
 * @see struct hash_table_t
 */
typedef PTR(struct hash_table_t) hash_table_t;

/**
 * @brief Represents a hash table entry
 */
struct hash_table_entry_t {
    generic_t key;
    generic_t value;
    bool owns_key;
    bool owns_value;
};

// ReSharper disable once CppDoxygenUnresolvedReference // TO DELETE
/**
 * @brief Hashes a generic value and returns an unsigned 32-bit integer.
 *
 * @param to_hash The value to hash
 * @returns The hash value, which will always be the same for the same value
 */
typedef unsigned int (*hasher_ft)(generic_t);

/**
 * @brief An identity hash.
 *
 * Hashes a pointer using a XOR fold
 *
 * @param to_hash The pointer to return a hash for
 * @return The hash of the pointer given
 */
unsigned int default_hasher(generic_t to_hash);

/**
 * @brief Creates a hash table and places it at `*new_table`.
 *
 * Allocates the memory for a hash table with zero entries, a given modulus, with given hashers,
 * comparators, copiers, and a custom allocator. To ensure the integrity of the return value, the
 * caller should check the returned code against SUCCESS and if the two are not equal, respond
 * accordingly.
 *
 * @param[out] new_table A pointer to the storage location of the pointer to the new hash table
 * @param modulus The modulus, and consequentially the amount of top level buckets, required to be a
 * power of two
 * @param allocator The custom allocator used by the hash table to dynamically allocate memory
 * @param hasher The function that hashes keys to map them to values
 * @param key_comparator The comparator that ensures key equality
 * @param key_copier Either the copier function for copying the key during the `copy_hash_table`
 * operation, or nullptr if copying isn't supported
 * @param value_copier Either the copier function for copying the value during the `copy_hash_table`
 * operation, or nullptr if copying isn't supported
 * @return The status code of the operation, either `SUCCESS`, `INVALID_ARGUMENT`,
 * `NULL_POINTER_ENCOUNTERED`, `CANT_ALLOCATE`, or `INVALID_ALLOCATOR`
 *
 * @note Thread safety hangs on the allocator being threadsafe.
 * @note Failed operations do not result in leaked memory (except for problems with custom
 * allocator).
 */
int create_hash_table_pro_ex(PTR(hash_table_t) new_table, unsigned int modulus, struct allocator_t allocator, hasher_ft hasher, comparator_ft key_comparator, copier_ft key_copier, copier_ft value_copier);
/**
 * @brief A convenience function for `create_hash_table_pro_ex` that fills in most fields, except for
 * `modulus`.
 *
 * @param[out] new_table A pointer to the storage location of the pointer to the new hash table
 * @param modulus The modulus, and consequentially the amount of top level buckets, required to be a
 * power of two
 * @return The status code of the operation, either `SUCCESS`, `INVALID_ARGUMENT`,
 * `NULL_POINTER_ENCOUNTERED`, or `CANT_ALLOCATE`.
 *
 * @note Not thread-safe as the system allocator is not thread-safe.
 */
int create_hash_table(PTR(hash_table_t) new_table, unsigned int modulus);
/**
 * @brief A convenience function for `create_hash_table_pro_ex` that fills in most fields, except for
 * `modulus` and `allocator`.
 *
 * @param[out] new_table A pointer to the storage location of the pointer to the new hash table
 * @param modulus The modulus, and consequentially the amount of top level buckets, required to be a
 * power of two
 * @param allocator The custom allocator used by the hash table to dynamically allocate memory
 * @return The status code of the operation, either `SUCCESS`, `INVALID_ARGUMENT`,
 * `NULL_POINTER_ENCOUNTERED`, `CANT_ALLOCATE`, or `INVALID_ALLOCATOR`
 *
 * @note Only as threadsafe as the passed in allocator.
 */
int create_hash_table_pro(PTR(hash_table_t) new_table, unsigned int modulus, struct allocator_t allocator);
/**
 * @brief A convenience function for `create_hash_table_pro_ex` that fills in most fields, except for
 * `modulus`, and the fill-in functions.
 *
 * @param[out] new_table A pointer to the storage location of the pointer to the new hash table
 * @param modulus The modulus, and consequentially the amount of top level buckets, required to be a
 * power of two
 * @param hasher The function that hashes keys to map them to values
 * @param key_comparator The comparator that ensures key equality
 * @param key_copier Either the copier function for copying the key during the `copy_hash_table`
 * operation, or nullptr if copying isn't supported
 * @param value_copier Either the copier function for copying the value during the `copy_hash_table`
 * operation, or nullptr if copying isn't supported
 * @return The status code of the operation, either `SUCCESS`, `INVALID_ARGUMENT`,
 * `NULL_POINTER_ENCOUNTERED`, `CANT_ALLOCATE`.
 *
 * @note Not thread-safe as the system allocator is not thread-safe.
 */
int create_hash_table_ex(PTR(hash_table_t) new_table, unsigned int modulus, hasher_ft hasher, comparator_ft key_comparator, copier_ft key_copier, copier_ft value_copier);
/**
 * @brief Copies a hash table.
 *
 * Copies a hash table and every entry within, taking ownership of the new key-value pairs and freeing
 * them upon table destruction. All new entries are owned, allocated, and destroyed by the hash table,
 * due to them not having existing references elsewhere. Ownership can still be transferred through
 * `remove_value_from_hash_table` to the caller.
 *
 * @param src The hash table to copy
 * @param[out] dest A pointer to the storage location of the pointer to the new hash table
 * @returns The status code of the operation, either `SUCCESS`, `NULL_POINTER_ENCOUNTERED`, or
 * `CANT_ALLOCATE`.
 *
 * @note Not threadsafe if underlying table allocator is not threadsafe.
 * @note Will fail to copy if the `create_hash_table_*` function was not supplied with non-null
 * copiers.
 */
int copy_hash_table(hash_table_t src, PTR(hash_table_t) dest);
/**
 * @brief Retrieves a value from the hash table using a key.
 *
 * Can execute in O(1) best case but in the case of many collisions it can degrade to O(n).
 *
 * @param src The table to retrieve the value from
 * @param key The key (or a pointer to the key)
 * @param dest[out] The storage location the return value should be placed at
 * @return The status code the operation returns, either `SUCCESS`, `NULL_POINTER_ENCOUNTERED`, or
 * `OUT_OF_BOUNDS`.
 *
 * @note `OUT_OF_BOUNDS` means the value is not present in the table.
 */
int get_value_from_hash_table(hash_table_t src, generic_t key, PTR(generic_t) dest);

/**
 * @brief Places a value in the hash table, possibly transferring ownership and responsibility
 *
 * Can execute in O(1) best case but in the case of many collisions it can degrade to O(n).
 *
 * @param dest The table to modify
 * @param key The key to place the new value at
 * @param value The value to place
 * @param owns_key Whether the hash table should handle the freeing of the key on table destruction
 * @param owns_value Whether the hash table should handle the freeing of the value on table
 * destruction
 * @return The status code the operation returns, either `SUCCESS`, `NULL_POINTER_ENCOUNTERED`, or
 * `CANT_ALLOCATE`.
 *
 * @note The table will never take ownership of a `nullptr`.
 * @note Not threadsafe.
 * @note The key and value should both use the same allocator as the table if you plan on transferring
 * ownership.
 */
int put_value_in_hash_table(hash_table_t dest, generic_t key, generic_t value, bool owns_key, bool owns_value);
/**
 * @brief Rehashes the table with a new modulus.
 *
 * Rehashes the table with a new modulus (that must be a power of two) to reduce collisions.
 *
 * @param table The table that will be rehashed and balanced
 * @param new_modulus The new modulus for the table
 * @return The status of the operation upon completion, either `SUCCESS`, `NULL_POINTER_ENCOUNTERED`,
 * `INVALID_ARGUMENT`, or `CANT_ALLOCATE`.
 *
 * @note Allocates a pointer buffer for storing buckets' pointers intermediately, so big tables may
 * cause problems.
 * @note New modulus must be a power of two.
 * @note Not threadsafe.
 * @note Failed operations will not modify the hash table.
 */
int rehash_hash_table(hash_table_t table, unsigned int new_modulus);

/**
 * @brief Removes a value from the hash table, optionally returning it.
 *
 * Removes a value from the hash table, freeing the memory allocated by the entry if necessary, or
 * optionally returning the value instead of freeing it, transferring ownership as a result if the
 * table has ownership of either the key or value. If the table does not own the key or value at the
 * entry, then said unowned key and value will not be freed upon removal nor the ownership passed to
 * the caller upon request. You can find the caller's ownership status of the key and value by
 * checking the `owns_key` or `owns_value`.
 *
 * @param src The table to remove the entry from
 * @param key The key pointing to the entry to remove
 * @param dest[out] The place to put the entry if the entry is to be returned instead of freed
 * @param return_value Whether to return the entry instead of freeing it outright
 * @return The status of the operation, which is either `SUCCESS`, `NULL_POINTER_ENCOUNTERED`,
 * or `OUT_OF_BOUNDS`.
 */
int remove_value_from_hash_table(hash_table_t src, generic_t key, PTR(struct hash_table_entry_t) dest, bool return_value);
/**
 * @brief Frees up the memory allocated to and owned by the hash table given.
 *
 * Frees up the memory allocated to and owned by the hash table given, using the `allocator.free`
 * function. If any key-value entries contain keys or values that are pointers to otherwise
 * inaccessible memory, and they are also owned by the table, they will be freed. It is user
 * responsibility to ensure the table knows what it owns so it can properly prevent memory leaks.
 *
 * @param table A reference to the table handle, the table being the one to free
 * @return The status of the operation, which is either `SUCCESS` or `NULL_POINTER_ENCOUNTERED`.
 *
 * @note Not threadsafe unless the underlying allocator is threadsafe.
 * @note The variable whose reference is passed will be set to nullptr to prevent use-after-free.
 * Attempted continued use will result in `NULL_POINTER_ENCOUNTERED` or a segfault.
 */
int free_hash_table(PTR(hash_table_t) table);

#ifdef __cplusplus
}
#endif

#endif //COLLECTIONS_HASH_TABLE_H
