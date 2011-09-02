set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_library(sockets MODULE sockets.c)

set_target_properties(sockets PROPERTIES
  PREFIX ""
)
