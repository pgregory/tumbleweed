set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

find_package(SFML 2 COMPONENTS window system graphics audio REQUIRED)

include_directories(${SFMLL_INCLUDE_DIR})

add_library(sfml MODULE sfml.cpp)

set_target_properties(sfml PROPERTIES
  PREFIX ""
)

target_link_libraries(sfml ${SFML_LIBRARIES})

install(TARGETS sfml LIBRARY DESTINATION "modules/sfml")
install(FILES sfml.st DESTINATION "modules/sfml")

