set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_library(simple1 MODULE simple1.c)

set_target_properties(simple1 PROPERTIES
  PREFIX ""
)
