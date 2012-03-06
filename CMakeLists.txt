set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
cmake_minimum_required ( VERSION 2.6 )

project (Tumbleweed)

option(TW_BUILD_TESTS "Build all of Tumbleweeds's unit tests." OFF)

set(shared_srcs 
  source/ffiprimitive.cpp
  source/filein.cpp
  source/interp.cpp
  source/lex.cpp
  source/memory.cpp
  source/names.cpp
  source/parser.cpp
  source/parser.cpp
  source/primitive.cpp
  source/sysprimitive.cpp
  source/unixio.cpp
)

set(headers
  source/env.h
  source/interp.h
  source/lex.h
  source/memory.h
  source/names.h
)
  
set(tw_srcs 
  source/st.cpp
)

set(init_srcs 
  source/initial.cpp
)

set(common_image_srcs
  bootstrap/basic.st
  bootstrap/collect.st
  bootstrap/ffi.st
  bootstrap/file.st
  bootstrap/graphics.st
  bootstrap/mag.st
  bootstrap/mult.st
  bootstrap/queen.st
  bootstrap/test.st
  bootstrap/iup.st
  bootstrap/browser.st
  bootstrap/workspace.st
  bootstrap/module.st
  bootstrap/httprequest.st
)

set(image_srcs ${common_image_srcs}
  bootstrap/tty.st
)

set(server_image_srcs ${common_image_srcs}
  bootstrap/server.st
)

find_package(LibFFI REQUIRED)

include_directories(${LIBFFI_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LIBFFI_LIBRARIES})

function(prefix_sources prefix sources)
  set(_TMP)
  foreach(i IN LISTS ${sources})
    list(APPEND _TMP ${prefix}/${i})
  endforeach()
  set(${sources} ${_TMP} PARENT_SCOPE)
endfunction(prefix_sources)

prefix_sources(${CMAKE_CURRENT_SOURCE_DIR} image_srcs)
prefix_sources(${CMAKE_CURRENT_SOURCE_DIR} server_image_srcs)

add_library(libtw ${shared_srcs})
set_target_properties(libtw PROPERTIES OUTPUT_NAME tw)

add_executable(initial ${init_srcs})
add_custom_command(
  OUTPUT systemImage
  COMMAND initial ${image_srcs}
  DEPENDS initial ${image_srcs}
)
add_custom_command(
  OUTPUT serverSystemImage
  COMMAND initial ${server_image_srcs}
  DEPENDS initial ${server_image_srcs}
)

add_executable(tw ${tw_srcs} systemImage serverSystemImage)

target_link_libraries(initial libtw ${LIBS})
target_link_libraries(tw libtw ${LIBS})

install(TARGETS tw RUNTIME DESTINATION .)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/systemImage DESTINATION .)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/serverSystemImage DESTINATION .)

# Testing
if(TW_BUILD_TESTS)
  enable_testing()
  find_package(GTest REQUIRED)
  include_directories(${GTEST_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/source)
  add_executable(memory_manager_test
    test/memory_test.cpp)
  target_link_libraries(memory_manager_test libtw ${GTEST_BOTH_LIBRARIES} ${LIBS})
  set(memory_manager_test_args "")
  GTEST_ADD_TESTS(memory_manager_test "${memory_manager_test_args}" test/memory_test.cpp)
endif(TW_BUILD_TESTS)

# Modules.
add_subdirectory(ffi_test)
add_subdirectory(sockets)
add_subdirectory(linenoise)
add_subdirectory(trex)
add_subdirectory(events)


