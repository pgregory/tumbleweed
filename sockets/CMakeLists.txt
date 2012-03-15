add_library(sockets MODULE sockets.c)

set_target_properties(sockets PROPERTIES
  PREFIX ""
)

target_link_libraries(sockets ws2_32)

install(TARGETS sockets LIBRARY DESTINATION "modules/sockets")
install(FILES sockets.st DESTINATION "modules/sockets")

