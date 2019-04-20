set(CMAKE_C_FLAGS "${CMAKE_C_FALGS} -c -m64 -std=gnu99 -Wall")

add_library(${PROJECT_NAME}_common STATIC
    opensdl_blocks.c
    opensdl_message.c)

target_include_directories(${PROJECT_NAME}_common PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
