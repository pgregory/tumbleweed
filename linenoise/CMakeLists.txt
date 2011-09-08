set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules/linenoise")

add_library(linenoise MODULE linenoise.c linenoise.h)

set_target_properties(linenoise PROPERTIES
  PREFIX ""
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/linenoise.st" "${CMAKE_BINARY_DIR}/modules/linenoise" COPYONLY)
