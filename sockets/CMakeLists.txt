add_library(sockets MODULE sockets.c)

set_target_properties(sockets PROPERTIES
  PREFIX ""
)

if(WIN32)
  target_link_libraries(sockets ws2_32)
endif(WIN32)

install(TARGETS sockets LIBRARY DESTINATION "modules/sockets")
install(FILES sockets.st DESTINATION "modules/sockets")

