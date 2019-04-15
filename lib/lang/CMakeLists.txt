set(CMAKE_C_FLAGS "${CMAKE_C_FALGS} -c -m64 -std=gnu99 -Wall")

add_library(${PROJECT_NAME}_plugin STATIC
    opensdl_plugin.c)

target_include_directories(${PROJECT_NAME}_plugin PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR})

set_target_properties(${PROJECT_NAME}_plugin PROPERTIES
    COMPILE_FLAGS ${CMAKE_C_FLAGS} -c -m64 -std=gnu99 -Wall)

add_library(${PROJECT_NAME}_c SHARED
    opensdl_c.c)

target_include_directories(${PROJECT_NAME}_c PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR})


set_target_properties(${PROJECT_NAME}_plugin PROPERTIES
    COMPILE_FLAGS "${CMAKE_C_FLAGS} -c -fPIC -m64 -std=gnu99 -Wall")

    