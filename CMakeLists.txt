set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
cmake_minimum_required ( VERSION 2.6 )

project (Tumbleweed)

option(TW_BUILD_TESTS "Build all of Tumbleweeds's unit tests." OFF)
option(TW_ENABLE_FFI "Enable FFI for native module support." ON)
option(TW_SMALLINTEGER_AS_OBJECT "Disable the special handling of small integers, use objects instead." OFF)

if(TW_SMALLINTEGER_AS_OBJECT)
  add_definitions(-DTW_SMALLINTEGER_AS_OBJECT)
endif(TW_SMALLINTEGER_AS_OBJECT)

#find_package( Boost 1.36.0 )
#if(Boost_FOUND)
#   include_directories(${Boost_INCLUDE_DIRS})
#endif()

set(shared_srcs 
  source/filein.c
  source/interp.c
  source/lex.c
  source/memory.c
  source/names.c
  source/parser.c
  source/parser.c
  source/primitive.c
  source/sysprimitive.c
  source/unixio.c
)

set(headers
  source/env.h
  source/interp.h
  source/lex.h
  source/memory.h
  source/names.h
)
  
set(tw_srcs 
  source/st.c
)

set(init_srcs 
  source/initial.c
)

set(common_image_srcs
  bootstrap/basic.st
  bootstrap/collect.st
  bootstrap/file.st
  bootstrap/mag.st
  bootstrap/graphics.st
  bootstrap/mult.st
  bootstrap/queen.st
  bootstrap/module.st
  bootstrap/httprequest.st
  bootstrap/exception.st
  bootstrap/test.st
  bootstrap/time.st
  bootstrap/bench.st
  bootstrap/morphic.st

  bootstrap/test_collect.st
  bootstrap/test_morphic.st
)

if(TW_ENABLE_FFI)
  find_package(LibFFI REQUIRED)
  set(shared_srcs ${shared_srcs} source/ffiprimitive.c)
  set(ffi_image_srcs
      bootstrap/ffi.st
      bootstrap/iup.st
      bootstrap/browser.st
      bootstrap/workspace.st
      )
  add_definitions(-DTW_ENABLE_FFI)
  if(WIN32)
    add_definitions(-DFFI_BUILDING)
  endif(WIN32)
endif(TW_ENABLE_FFI)

set(image_srcs ${common_image_srcs} ${ffi_image_srcs}
  bootstrap/tty.st
)

#set(server_image_srcs ${common_image_srcs} ${ffi_image_srcs}
#  bootstrap/server.st
#)


include_directories(${LIBFFI_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LIBFFI_LIBRARIES})

if(WIN32)
    add_definitions(-DSO_EXT="dll" -D_CRT_SECURE_NO_WARNINGS)
else(WIN32)
  add_definitions(-DSO_EXT="so")
  find_library(TW_EDITLINE_LIBRARY "edit")
  find_path(TW_EDITLINE_INCLUDE_DIR "editline/readline.h")
  include_directories(${TW_EDITLINE_INCLUDE_DIR})
endif(WIN32)

function(prefix_sources prefix sources)
  set(_TMP)
  foreach(i IN LISTS ${sources})
    list(APPEND _TMP ${prefix}/${i})
  endforeach()
  set(${sources} ${_TMP} PARENT_SCOPE)
endfunction(prefix_sources)

prefix_sources(${CMAKE_CURRENT_SOURCE_DIR} image_srcs)
#prefix_sources(${CMAKE_CURRENT_SOURCE_DIR} server_image_srcs)

add_library(libtw ${shared_srcs} ${headers})
set_target_properties(libtw PROPERTIES OUTPUT_NAME tw)
source_group(Headers FILES ${headers})
source_group(Source FILES ${shared_srcs})

if(NOT WIN32)
  list(APPEND LIBS "dl" ${TW_EDITLINE_LIBRARY})
endif(NOT WIN32)

add_executable(initial ${shared_srcs} ${init_srcs} ${image_srcs})
source_group(Bootstrap FILES ${image_srcs})
set_target_properties(initial PROPERTIES COMPILE_DEFINITIONS TW_IS_INITIAL=1)
add_custom_command(
  OUTPUT systemImage
  COMMAND initial ${image_srcs}
  DEPENDS initial ${image_srcs}
)
#add_custom_command(
#  OUTPUT serverSystemImage
#  COMMAND initial ${server_image_srcs}
#  DEPENDS initial ${server_image_srcs}
#)

add_executable(tw ${shared_srcs} ${headers} ${tw_srcs} systemImage)
if(NOT MSVC AND NOT APPLE)
  set_target_properties(tw PROPERTIES LINK_FLAGS "-Wl,-Map=tw.map")
endif(NOT MSVC AND NOT APPLE)
source_group(Headers FILES ${headers})
source_group(Source FILES ${shared_srcs})

target_link_libraries(initial ${LIBS})
target_link_libraries(tw ${LIBS})


install(TARGETS tw RUNTIME DESTINATION .)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/systemImage DESTINATION .)
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/serverSystemImage DESTINATION .)

# Testing
if(TW_BUILD_TESTS)
  enable_testing()
  find_package(GTest REQUIRED)
  include_directories(${GTEST_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/source)
  add_definitions(-DTW_UNIT_TESTS)
  add_executable(memory_manager_test
    test/memory_test.cpp)
  target_link_libraries(memory_manager_test libtw ${GTEST_BOTH_LIBRARIES} ${LIBS})
  set(memory_manager_test_args "")
  GTEST_ADD_TESTS(memory_manager_test "${memory_manager_test_args}" test/memory_test.cpp)
endif(TW_BUILD_TESTS)

# Modules.
add_subdirectory(ffi_test)
add_subdirectory(sockets)
add_subdirectory(sfml)
#add_subdirectory(linenoise)
#add_subdirectory(trex)
if(NOT WIN32)
  add_subdirectory(events)
endif(NOT WIN32)


