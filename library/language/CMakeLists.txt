add_library(${PROJECT_NAME}_c SHARED
    opensdl_c.c)

target_include_directories(${PROJECT_NAME}_c PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR})

set_target_properties(${PROJECT_NAME}_c PROPERTIES
    POSITION_INDEPENDENT_CODE 1)

target_link_libraries(${PROJECT_NAME}_c PRIVATE
    ${PROJECT_NAME}_common)

set_source_files_properties(opensdl_c.c PROPERTIES
    COMPILE_FLAGS "-Wno-format-security")

