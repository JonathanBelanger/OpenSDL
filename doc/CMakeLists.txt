find_program(TEXI2PDF_EXECUTABLE texi2pdf)

set(info_in ${CMAKE_CURRENT_SOURCE_DIR}/opensdl.texi)
set(info_pdf ${CMAKE_CURRENT_BINARY_DIR}/opensdl.pdf)
#set(info_html ${CMAKE_CURRENT_BINARY_DIR}/opensdl.html)
add_custom_command(OUTPUT ${info_pdf}
  COMMAND ${TEXI2PDF_EXECUTABLE} --clean --pdf --output=${info_pdf} ${info_in}
  DEPENDS ${info_in}
  COMMENT "Creating PDF file ${info_pdf}"
  VERBATIM)
#add_custom_command(OUTPUT ${info_html}
#  COMMAND ${TEXI2PDF_EXECUTABLE} --clean --html --output=${info_html} ${info_in}
#  DEPENDS ${info_in}
#  COMMENT "Creating PDF file ${info_html}"
#  VERBATIM)

add_custom_target(documentation ALL DEPENDS ${info_pdf})
# ${info_html})
