add_library(simple1 MODULE simple1.c)

set_target_properties(simple1 PROPERTIES
  PREFIX ""
)

install(TARGETS simple1 LIBRARY DESTINATION .)
