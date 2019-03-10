find_program(TEXI2PDF_EXECUTABLE texi2pdf)

set(info_in ${CMAKE_CURRENT_SOURCE_DIR}/opensdl.texi)
set(info_out ${CMAKE_CURRENT_BINARY_DIR}/opensdl.pdf)
add_custom_command(OUTPUT ${info_out}
  COMMAND ${TEXI2PDF_EXECUTABLE} --clean --pdf --output=${info_out} ${info_in}
  DEPENDS ${info_in}
  COMMENT "Creating PDF file ${info_out}"
  VERBATIM)

add_custom_target(documentation ALL DEPENDS ${info_out})
