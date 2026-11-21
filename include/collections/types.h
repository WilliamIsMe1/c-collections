#ifndef COLLECTIONS_TYPES_H
#define COLLECTIONS_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

/**
 * @brief An enum type used to create integer values for status codes.
 *
 * A type used to automatically assign numbers to certain errors. For example, `SUCCESS` means that
 * the operation went well. `NULL_POINTER_ENCOUNTERED` means that you passed in a `nullptr` somewhere
 * you shouldn't have, or that a `nullptr` was found in a place it shouldn't have been. There are
 * other enums for other errors, but if you can't find the one you want for your data structure and
 * need to extend the type, you can create your own codes with the pattern:
 * <pre>
 * typedef enum {
 *     NEW_ERROR_1 = ENUM_ERROR_T_SENTINEL_THAT_IS_ALSO_SIZE_OF_ENUM,
 *     NEW_ERROR_2,
 *     NEW_ERROR_3,
 *     ERROR_EXTENSION_T_SENTINEL // DON'T USE AS AN ERROR CODE
 * } error_extension_t;
 * </pre>
 *
 * Sorry for the ugly name.
 */
typedef enum {
    /** @brief The operation was successful. */
    SUCCESS,
    /** @brief A `nullptr` was encountered in a bad place. */
    NULL_POINTER_ENCOUNTERED,
    /** @brief `malloc`, `calloc`, or `realloc` returned `nullptr`. */
    CANT_ALLOCATE,
    /** @brief The argument passed was invalid. */
    INVALID_ARGUMENT,
    /** @brief The index was not in the array. */
    OUT_OF_BOUNDS,
    /** @brief A value passed in was not of the correct enumeration. */
    INVALID_ENUM,
    /** @brief This operation is unsupported in this context. */
    UNSUPPORTED_OPERATION,
    /** @brief This allocator does not implement the correct functions. */
    INVALID_ALLOCATOR,
    /** @brief End value for extension of error_t. */
    ENUM_ERROR_T_SENTINEL_THAT_IS_ALSO_SIZE_OF_ENUM // DON'T USE AS AN ERROR CODE; it's for extending the error_t
} error_t;

/**
 * @brief A pointer to a variable of `type`.
 * @param type The type that this is a pointer to
 */
#define PTR(type) type *
/**
 * @brief A const pointer to a variable of `type`.
 * @param type The type that this is a constant pointer to
 */
#define CONST_PTR(type) const type *
/**
 * @brief A pointer to a constant of `type`.
 * @param type The type that this is a pointer to
 */
#define PTR_CONST(type) type * const
/**
 * @brief A constant pointer to a constant of `type`.
 * @param type The type that this is a constant pointer to
 */
#define CONST_PTR_CONST(type) const type * const

/**
 * @brief An array of `type`.
 * @param type The type that this is an array of
 * @note This is to help distinguish arrays from pointers and can be completely left unused.
 */
#define ARR(type) type *

/** @brief A quick convenience string type. */
typedef ARR(char) string_t;
/** @brief A quick convenience constant string type. */
typedef CONST_PTR_CONST(char) const_string_t;

/**
 * @brief A constant list of constant strings that map enums to names.
 *
 * For use, just reference <pre>error_messages[error_code]</pre> to get a string for your logs.
 */
extern const_string_t const error_messages[ENUM_ERROR_T_SENTINEL_THAT_IS_ALSO_SIZE_OF_ENUM]; // EUUGH descriptive names can be UGLY sometimes

/** @brief A pointer to a generic value. */
typedef void* generic_t;
/** @brief A pointer to a specific range of memory. */
typedef void* memory_t;

/**
 * @brief Checks if `value` is a power of two.
 * @param value The value to verify log_2(val) % 1 == 0
 * @return Whether `value` is a power of two.
 */
static inline bool is_power_of_two(const unsigned int value) {
    return value != 0 && (value & value - 1) == 0;
}

/**
 * @brief A structure that represents a full and complete allocator system.
 *
 * @note Threadsafety is far from guaranteed.
 */
struct allocator_t {
    /**
     * @brief Allocates a block of memory of `len` bytes.
     * @param len The size of the new block in bytes
     * @return A pointer to the new memory
     *
     * @note If it fails to allocate, will return `nullptr`.
     * @note `len` of 0 is undefined behavior.
     */
    memory_t (*malloc ) (size_t len              );
    /**
     * @brief Allocates a zeroed block of memory of `len` elements of `size` bytes.
     * @param size The size of each element in bytes. Use the `sizeof` operator
     * @param len The length of the new array in elements
     * @return A pointer to the new memory
     *
     * @note If it fails to allocate, will return `nullptr`.
     * @note `len` of 0 or `size` of 0 is undefined behavior.
     * @
     */
    memory_t (*calloc ) (size_t size,  size_t len);
    /**
     * @brief Reallocates a block of memory to a different size, potentially changing the position
     * thereof.
     * @param ptr The pointer to the memory to resize
     * @param len The new length for the block of memory in bytes.
     * @return The pointer to the reallocated memory, potentially different than what was passed in
     *
     * @note If it fails to allocate, will return `nullptr` and will not change the current allocation.
     * @note `nullptr` or an invalid pointer will result in a segfault or undefined behavior.
     * @note `len` of 0 is undefined behavior.
     */
    memory_t (*realloc) (memory_t ptr, size_t len);
    /**
     * @brief Frees up a block of memory to be reallocated
     * @param ptr The memory to be freed and invalidated
     *
     * @note Can cause segfaults on double free.
     * @note Remember that use after free was such a problem someone reinvented the wheel to try to
     * combat it. We call the "solution" Rust. You're here cause you don't like Rust or prefer not to
     * use it in this case. So please be careful.
     */
    void     (*free   ) (memory_t ptr            );
};

/**
 * The default system allocator from <stdlib.h>, as a `struct allocator_t`.
 */
const extern struct allocator_t default_allocator;

// ReSharper disable CppDoxygenUnresolvedReference // TO REMOVE
/**
 * @brief A comparator function type.
 *
 * Represents a comparator that returns an integer representing the difference between two generic
 * values. Will return a negative value if a < b, a positive value if a > b, and zero if a is
 * equivalent to b, which does not necessarily mean that a == b or that a and b are the same.
 *
 * @param a The first value to compare
 * @param b The second value to compare
 * @return The result of the comparison between a and b
 *
 * @note if cmp(a,b) is negative, cmp(b,a) should be positive, and if cmp(a,b) == 0, then cmp(b,a)
 * should also be 0.
 */
typedef int (*comparator_ft)(generic_t, generic_t);
/**
 * @brief A generic copy function type.
 *
 * Represents a copier meant to take an object and duplicate it in a deep copy, allocating new
 * memory. As this operation is one that can fail, it follows the same pattern of returning a code
 * and using a parameter for a return value. It takes a generic value to copy and an allocator, with
 * which it can perform a deep copy.
 *
 * @param to_copy The value to copy
 * @param allocator The allocator to use
 * @param return_val The reference to the variable in which to place the copy
 * @return The status code of the operation, which can be whatever the user defines.
 *
 * @note Only as threadsafe as the implementator decides to be.
 */
typedef int (*copier_ft)(generic_t,struct allocator_t,PTR(generic_t) return_val);
// ReSharper enable CppDoxygenUnresolvedReference // TO REMOVE

/** @brief The default comparator that just compares the addresses. */
int default_comparator(generic_t a, generic_t b);

#ifdef __cplusplus
}
#endif

#endif
