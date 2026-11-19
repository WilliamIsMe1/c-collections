cmake_minimum_required(VERSION 4.3)
project(include/collections C)

set(CMAKE_C_STANDARD 23)

add_library(collections
        src/types.c
        src/hash_table.c
        src/vector.c
        src/buffer.c
        src/memmove_safe.c
)

target_include_directories(collections PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_executable(collections_tests
        tests/main.c
)

target_link_libraries(collections_tests PRIVATE collections)