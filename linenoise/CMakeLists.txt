add_library(linenoise MODULE 
  linenoise/linenoise.c 
  linenoise/linenoise.h
  linenoise/utf8.c
  linenoise/utf8.h)

set_target_properties(linenoise PROPERTIES
  PREFIX ""
)

install(TARGETS linenoise LIBRARY DESTINATION "modules/linenoise")
install(FILES linenoise.st DESTINATION "modules/linenoise")
