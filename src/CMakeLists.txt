set(CMAKE_C_FLAGS "${CMAKE_C_FALGS} -c -m64 -std=gnu99 -Wall")

find_package(BISON REQUIRED)
set(BISON_OUTPUT opensdl_parser.c)
if(BISON_FOUND)
    add_custom_command(
        OUTPUT ${BISON_OUTPUT}
        COMMAND ${BISON_EXECUTABLE}
            -v --report=lookahead -d -Lc
            --output=${BISON_OUTPUT}
            ${CMAKE_SOURCE_DIR}/opensdl_parser.y
        COMMENT "Generating opensdl_parser.c")
endif()

find_package(FLEX REQUIRED)
set(FLEX_OUTPUT opensdl_lexical.c)
if(FLEX_FOUND)
    add_custom_command(
        OUTPUT ${FLEX_OUTPUT}
        COMMAND ${FLEX_EXECUTABLE}
            --output=${FLEX_OUTPUT}
            ${CMAKE_SOURCE_DIR}/opensdl_lexical.l
        COMMENT "Generating opensdl_lexical.c")
endif()

add_library(${PROJECT_NAME} STATIC
    opensdl_actions.c
    opensdl_blocks.c
    opensdl_listing.c
    opensdl_message.c
    opensdl_utility.c
    opensdl_parser.c
    opensdl_lexical.c)

add_executable(opensdl
    opensdl_main.c)
