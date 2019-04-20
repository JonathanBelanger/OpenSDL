
add_executable(${PROJECT_NAME}
    opensdl_main.c)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${PROJECT_NAME}_common
    ${PROJECT_NAME}_lexical
    ${PROJECT_NAME}_utility
    -largp)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR})
