set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules/sockets")

add_library(sockets MODULE sockets.c)

set_target_properties(sockets PROPERTIES
  PREFIX ""
)

cm_export_file("${CMAKE_CURRENT_SOURCE_DIR}/sockets.st" "${CMAKE_BINARY_DIR}/modules/sockets")
