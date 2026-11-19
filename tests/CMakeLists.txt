cmake_minimum_required(VERSION 4.3)
project(collections_tests C)

set(CMAKE_C_STANDARD 23)

add_subdirectory(.. collections-build)

add_executable(collections_tests
        main.c
)

target_link_libraries(collections_tests PRIVATE collections)

enable_testing()

add_test(NAME collections_tests COMMAND collections_tests)