set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
cmake_minimum_required ( VERSION 2.6 )

project (Tumbleweed)

set(shared_srcs 
  source/ffiprimitive.c
  source/filein.c
  source/interp.c
  source/lex.c
  source/memory.c
  source/names.c
  source/parser.c
  source/news.c
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
  
set(tw_srcs ${shared_srcs}
  source/st.c
)

set(init_srcs ${shared_srcs}
  source/initial.c
)

set(image_srcs
  bootstrap/basic.st
  bootstrap/collect.st
  bootstrap/ffi.st
  bootstrap/file.st
  bootstrap/graphics.st
  bootstrap/mag.st
  bootstrap/mult.st
  bootstrap/queen.st
  bootstrap/test.st
  bootstrap/tty.st
  bootstrap/iup.st
  bootstrap/browser.st
)

find_package(LibFFI REQUIRED)
find_package(LibEdit REQUIRED)

include_directories(${LIBFFI_INCLUDE_DIRS} ${LIBEDIT_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LIBFFI_LIBRARIES} ${LIBEDIT_LIBRARIES})

function(prefix_sources prefix sources)
  set(_TMP)
  foreach(i IN LISTS ${sources})
    list(APPEND _TMP ${prefix}/${i})
  endforeach()
  set(${sources} ${_TMP} PARENT_SCOPE)
endfunction(prefix_sources)

prefix_sources(${CMAKE_CURRENT_SOURCE_DIR} image_srcs)

add_executable(initial ${init_srcs})
add_custom_command(
  OUTPUT systemImage
  COMMAND initial ${image_srcs}
  DEPENDS initial ${image_srcs}
)

add_executable(tw ${tw_srcs} systemImage)

target_link_libraries(initial ${LIBS})
target_link_libraries(tw ${LIBS})

add_subdirectory(ffi_test)


