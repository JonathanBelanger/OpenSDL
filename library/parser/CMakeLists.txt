
find_package(BISON REQUIRED)
set(BISON_OUTPUT opensdl_parser.c)
if(BISON_FOUND)
    add_custom_command(
        OUTPUT ${BISON_OUTPUT}
        COMMAND ${BISON_EXECUTABLE}
            --report=lookahead -d -Lc
            --output=${BISON_OUTPUT}
            ${CMAKE_SOURCE_DIR}/library/parser/opensdl_parser.y
        COMMENT "Generating opensdl_parser.c")
endif()

find_package(FLEX REQUIRED)
set(FLEX_C_OUTPUT opensdl_lexical.c)
set(FLEX_H_OUTPUT opensdl_lexical.h)
if(FLEX_FOUND)
    add_custom_command(
        OUTPUT ${FLEX_C_OUTPUT}
        COMMAND ${FLEX_EXECUTABLE}
            -o ${FLEX_C_OUTPUT}
            --header-file=${FLEX_H_OUTPUT}
            ${CMAKE_SOURCE_DIR}/library/parser/opensdl_lexical.l
        COMMENT "Generating opensdl_lexical.c")
endif()

add_library(${PROJECT_NAME}_lexical STATIC
    opensdl_parser.c
    opensdl_lexical.c)

target_include_directories(${PROJECT_NAME}_lexical PUBLIC
    ${PROJECT_SOURCE_DIR}/include)

set_source_files_properties(opensdl_parser.c PROPERTIES
    COMPILE_FLAGS "-Wno-format-security")

