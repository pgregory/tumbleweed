find_package(FLTK REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(${FLTK_INCLUDE_DIR})

add_library(fltk MODULE fltk.cpp)

set_target_properties(fltk PROPERTIES
  PREFIX ""
)

target_link_libraries(fltk ${FLTK_LIBRARIES})

install(TARGETS fltk LIBRARY DESTINATION "modules/fltk")
install(FILES fltk.st DESTINATION "modules/fltk")

