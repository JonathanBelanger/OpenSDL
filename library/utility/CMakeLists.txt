set(CMAKE_C_FLAGS "${CMAKE_C_FALGS} -c -m64 -std=gnu99 -Wall")

add_library(${PROJECT_NAME}_utility STATIC
    opensdl_actions.c
    opensdl_listing.c
    opensdl_plugin.c
    opensdl_utility.c)

target_include_directories(${PROJECT_NAME}_utility PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR})

set_source_files_properties(opensdl_listing.c PROPERTIES
    COMPILE_FLAGS "-Wno-format-overflow")

set_source_files_properties(opensdl_plugin.c PROPERTIES
    COMPILE_FLAGS "-Wno-format-overflow -Wno-unused-result")


