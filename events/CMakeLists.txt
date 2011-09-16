add_library(events MODULE events.c)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libev-4.04)

set_target_properties(events PROPERTIES
  PREFIX ""
)

install(TARGETS events LIBRARY DESTINATION "modules/events")
install(FILES events.st DESTINATION "modules/events")
