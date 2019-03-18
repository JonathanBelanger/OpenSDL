
add_executable(opensdl
    opensdl_actions.c
    opensdl_listing.c
    opensdl_main.c)

target_link_libraries(opensdl PRIVATE
    ${PROJECT_NAME}_util
    ${PROJECT_NAME}_lexical
    ${PROJECT_NAME}_lang
    -largp)

target_include_directories(opensdl PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
