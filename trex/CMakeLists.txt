add_library(trex MODULE trex.c trex.h)

set_target_properties(trex PROPERTIES
  PREFIX ""
)

install(TARGETS trex LIBRARY DESTINATION "modules/trex")
install(FILES trex.st DESTINATION "modules/trex")
