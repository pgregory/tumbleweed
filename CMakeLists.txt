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
  source/filein.cpp
  source/interp.cpp
  source/lex.cpp
  source/objmemory.cpp
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
  source/objmemory.h
  source/names.h
)
  
set(tw_srcs 
  source/st.cpp
)

set(init_srcs 
  source/initial.cpp
)


if(TW_ENABLE_FFI)
  find_package(LibFFI REQUIRED)
  set(shared_srcs ${shared_srcs} source/ffiprimitive.cpp)
  add_definitions(-DTW_ENABLE_FFI)
  if(WIN32)
    add_definitions(-DFFI_BUILDING)
  endif(WIN32)
endif(TW_ENABLE_FFI)

include_directories(${LIBFFI_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LIBFFI_LIBRARIES})

if(WIN32)
    add_definitions(-DSO_EXT="dll" -D_CRT_SECURE_NO_WARNINGS)
else(WIN32)
  add_definitions(-DSO_EXT="so")
endif(WIN32)

function(prefix_sources prefix sources)
  set(_TMP)
  foreach(i IN LISTS ${sources})
    list(APPEND _TMP ${prefix}/${i})
  endforeach()
  set(${sources} ${_TMP} PARENT_SCOPE)
endfunction(prefix_sources)

add_subdirectory(thirdparty/glfw)
include_directories(thirdparty/glfw/include)

add_subdirectory(thirdparty/linenoise)
include_directories(thirdparty/linenoise)

set(nanovg_srcs
  thirdparty/nanovg/src/nanovg.c
)

include_directories(thirdparty/nanovg/src)
add_library(nanovg STATIC ${nanovg_srcs})

add_library(tumbleweed ${shared_srcs} ${headers})
source_group(Headers FILES ${headers})
source_group(Source FILES ${shared_srcs})

add_executable(initial ${init_srcs} ${image_srcs})
source_group(Bootstrap FILES ${image_srcs})

add_executable(tw ${tw_srcs} systemImage)
if(NOT MSVC)
  set_target_properties(tw PROPERTIES LINK_FLAGS "-Wl")
endif(NOT MSVC)
source_group(Headers FILES ${headers})
source_group(Source FILES ${shared_srcs})

target_link_libraries(initial ${LIBS} linenoise glfw tumbleweed ${GLFW_LIBRARIES} nanovg)
target_link_libraries(tw ${LIBS} linenoise glfw tumbleweed ${GLFW_LIBRARIES} nanovg)

install(TARGETS tw RUNTIME DESTINATION .)
install(FILES ${CMAKE_SOURCE_DIR}/systemImage DESTINATION .)
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/serverSystemImage DESTINATION .)

# Testing
if(TW_BUILD_TESTS)
  # Enable ExternalProject CMake module
  include(ExternalProject)

  # Download and install GoogleTest
  ExternalProject_Add(
      gtest
      URL https://googletest.googlecode.com/files/gtest-1.7.0.zip
      PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
      # Disable install step
      INSTALL_COMMAND ""
  )

  # Create a libgtest target to be used as a dependency by test programs
  add_library(libgtest IMPORTED STATIC GLOBAL)
  add_dependencies(libgtest gtest)

  # Set gtest properties
  ExternalProject_Get_Property(gtest source_dir binary_dir)
  set_target_properties(libgtest PROPERTIES
      "IMPORTED_LOCATION" "${binary_dir}/libgtest.a"
      "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
  #    "INTERFACE_INCLUDE_DIRECTORIES" "${source_dir}/include"
  )
  # I couldn't make it work with INTERFACE_INCLUDE_DIRECTORIES
  include_directories("${source_dir}/include")

  # Download and install GoogleMock
  ExternalProject_Add(
      gmock
      URL https://googlemock.googlecode.com/files/gmock-1.7.0.zip
      PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gmock
      # Disable install step
      INSTALL_COMMAND ""
  )

  # Create a libgmock target to be used as a dependency by test programs
  add_library(libgmock IMPORTED STATIC GLOBAL)
  add_dependencies(libgmock gmock)

  # Set gmock properties
  ExternalProject_Get_Property(gmock source_dir binary_dir)
  set_target_properties(libgmock PROPERTIES
      "IMPORTED_LOCATION" "${binary_dir}/libgmock.a"
      "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
  #    "INTERFACE_INCLUDE_DIRECTORIES" "${source_dir}/include"
  )
  # I couldn't make it work with INTERFACE_INCLUDE_DIRECTORIES
  include_directories("${source_dir}/include")

  enable_testing()
  #find_package(GTest REQUIRED)
  include_directories(${GTEST_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/source)
  add_definitions(-DTW_UNIT_TESTS)
  add_executable(memory_manager_test test/memory_test.cpp test/main.cpp source/objmemory.cpp)
  target_link_libraries(memory_manager_test libgtest)
  set(memory_manager_test_args "")
  add_test(memory_manager_test memory_manager_test)
  #GTEST_ADD_TESTS(memory_manager_test "${memory_manager_test_args}" test/memory_test.cpp)
endif(TW_BUILD_TESTS)

# Modules.
#add_subdirectory(ffi_test)
#add_subdirectory(sockets)
#add_subdirectory(sfml)
#add_subdirectory(linenoise)
#add_subdirectory(trex)
#if(NOT WIN32)
#  add_subdirectory(events)
#endif(NOT WIN32)

#add_subdirectory(fltk)

