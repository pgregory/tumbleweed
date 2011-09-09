add_library(sockets MODULE sockets.c)

set_target_properties(sockets PROPERTIES
  PREFIX ""
)

install(TARGETS sockets LIBRARY DESTINATION "modules/sockets")
install(FILES sockets.st DESTINATION "modules/sockets")
