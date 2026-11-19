cmake_minimum_required(VERSION 4.3)
project(collections C)

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